/**
 * @file encryption.cpp
 * @brief Implementation of encryption helpers used to stream encrypt/decrypt data.
 *
 * The implementation uses libsodium when available and provides chunked AEAD encryption
 * with XChaCha20-Poly1305.
 */

#include "encryption.h"
#include <QDebug>
#include <QtGlobal>
#include <QtEndian>

#ifdef HAVE_LIBSODIUM
#include <sodium.h>
#endif

bool Encryption::isAvailable()
{
#ifdef HAVE_LIBSODIUM
    static bool initialized = (sodium_init() >= 0);
    return initialized;
#else
    return false;
#endif
}

#ifdef HAVE_LIBSODIUM
namespace {
static constexpr size_t NONCE_BYTES = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
static constexpr size_t KEY_BYTES = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
static constexpr size_t TAG_BYTES = crypto_aead_xchacha20poly1305_ietf_ABYTES;
}

bool Encryption::encryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress)
{
    if (!isAvailable()) return false;
    if ((size_t)key.size() < KEY_BYTES) {
        qWarning() << "Key too short for XChaCha20-Poly1305";
        return false;
    }

    const unsigned char *keyPtr = reinterpret_cast<const unsigned char*>(key.constData());
    // Write header: magic 'GSE1' (4 bytes), version (1 byte), chunkSize (uint32 LE)
    const QByteArray magic = QByteArray::fromRawData("GSE1", 4);
    quint8 version = 1;
    quint32 cs_le = qToLittleEndian(static_cast<quint32>(params.chunkSize));
    if (out.write(magic.constData(), magic.size()) != (qint64)magic.size()) return false;
    if (out.write(reinterpret_cast<const char*>(&version), 1) != 1) return false;
    if (out.write(reinterpret_cast<const char*>(&cs_le), sizeof(cs_le)) != (qint64)sizeof(cs_le)) return false;

    qint64 total = -1;
    if (in.isSequential() == false) {
        total = in.size();
    }

    const int chunkSize = params.chunkSize;
    QByteArray buf;
    buf.resize(chunkSize);
    qint64 processed = 0;

    while (!in.atEnd()) {
        qint64 read = in.read(buf.data(), chunkSize);
        if (read < 0) return false;
        QByteArray plaintext = QByteArray::fromRawData(buf.constData(), static_cast<int>(read));

        // generate nonce
        unsigned char nonce[NONCE_BYTES];
        randombytes_buf(nonce, NONCE_BYTES);

        // allocate ciphertext buffer (plaintext + TAG)
        QByteArray cipher;
        cipher.resize(read + TAG_BYTES);
        unsigned long long cipher_len = 0;

        if (crypto_aead_xchacha20poly1305_ietf_encrypt(reinterpret_cast<unsigned char*>(cipher.data()), &cipher_len,
                                                       reinterpret_cast<const unsigned char*>(plaintext.constData()), (unsigned long long)read,
                                                       NULL, 0,
                                                       NULL,
                                                       nonce,
                                                       keyPtr) != 0) {
            qWarning() << "Encryption failed";
            return false;
        }

        // Write chunk length (uint32 little-endian), then nonce, then ciphertext bytes
        quint32 ctLen = static_cast<quint32>(cipher_len);
        quint32 le = qToLittleEndian(ctLen);
        if (out.write(reinterpret_cast<const char*>(&le), sizeof(le)) != sizeof(le)) return false;
        if (out.write(reinterpret_cast<const char*>(nonce), NONCE_BYTES) != (qint64)NONCE_BYTES) return false;
        if (out.write(cipher.constData(), static_cast<qint64>(cipher_len)) != (qint64)cipher_len) return false;

        processed += read;
        if (progress) progress(processed, total);
    }

    return true;
}

bool Encryption::decryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress)
{
    if (!isAvailable()) return false;
    if ((size_t)key.size() < KEY_BYTES) {
        qWarning() << "Key too short for XChaCha20-Poly1305";
        return false;
    }

    const unsigned char *keyPtr = reinterpret_cast<const unsigned char*>(key.constData());
    qint64 total = -1;
    qint64 processed = 0;

    // Read and validate header: magic (4 bytes), version (1 byte), chunkSize (uint32 LE)
    char magicBuf[4];
    if (in.read(magicBuf, 4) != 4) {
        qWarning() << "Failed to read magic/header";
        return false;
    }
    QByteArray magic = QByteArray::fromRawData(magicBuf, 4);
    if (magic != QByteArray::fromRawData("GSE1", 4)) {
        qWarning() << "Bad or unsupported magic header" << magic;
        return false;
    }
    quint8 version = 0;
    if (in.read(reinterpret_cast<char*>(&version), 1) != 1) {
        qWarning() << "Failed to read header version";
        return false;
    }
    if (version != 1) {
        qWarning() << "Unsupported encryption format version" << version;
        return false;
    }
    quint32 cs_le = 0;
    if (in.read(reinterpret_cast<char*>(&cs_le), sizeof(cs_le)) != (qint64)sizeof(cs_le)) {
        qWarning() << "Failed to read header chunk size";
        return false;
    }
    quint32 headerChunkSize = qFromLittleEndian(cs_le);

    qint64 total = -1;
    qint64 processed = 0;

    while (!in.atEnd()) {
        // read 4-byte length
        quint32 le = 0;
        qint64 r = in.read(reinterpret_cast<char*>(&le), sizeof(le));
        if (r == 0) break; // EOF clean
        if (r != sizeof(le)) {
            qWarning() << "Truncated header while reading chunk length";
            return false;
        }
        quint32 ctLen = qFromLittleEndian(le);
        if (ctLen == 0) continue;

        if (ctLen > static_cast<quint32>(headerChunkSize + TAG_BYTES)) {
            qWarning() << "Chunk too large" << ctLen;
            return false;
        }

        QByteArray nonce;
        nonce.resize(NONCE_BYTES);
        if (in.read(nonce.data(), NONCE_BYTES) != (qint64)NONCE_BYTES) {
            qWarning() << "Failed to read nonce";
            return false;
        }

        QByteArray cipher;
        cipher.resize(ctLen);
        if (in.read(cipher.data(), ctLen) != (qint64)ctLen) {
            qWarning() << "Truncated ciphertext";
            return false;
        }

        QByteArray plain;
        plain.resize(ctLen); // will shrink after decrypt
        unsigned long long plain_len = 0;

        if (crypto_aead_xchacha20poly1305_ietf_decrypt(reinterpret_cast<unsigned char*>(plain.data()), &plain_len,
                                                       NULL,
                                                       reinterpret_cast<const unsigned char*>(cipher.constData()), (unsigned long long)ctLen,
                                                       NULL, 0,
                                                       reinterpret_cast<const unsigned char*>(nonce.constData()),
                                                       keyPtr) != 0) {
            qWarning() << "Decryption failed (auth)";
            return false;
        }

        plain.resize(static_cast<int>(plain_len));
        if (out.write(plain.constData(), plain.size()) != (qint64)plain.size()) return false;

        processed += plain.size();
        if (progress) progress(processed, total);
    }

    return true;
}
#else
// Fallback stubs when libsodium not found
bool Encryption::encryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress)
{
    Q_UNUSED(key)
    Q_UNUSED(params)
    Q_UNUSED(progress)
    Q_UNUSED(in)
    Q_UNUSED(out)
    qWarning() << "Encryption backend not available: encryption is a no-op in this build.";
    return false;
}

bool Encryption::decryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress)
{
    Q_UNUSED(key)
    Q_UNUSED(params)
    Q_UNUSED(progress)
    Q_UNUSED(in)
    Q_UNUSED(out)
    qWarning() << "Encryption backend not available: decryption is a no-op in this build.";
    return false;
}
#endif
