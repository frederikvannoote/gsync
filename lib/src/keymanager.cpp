/**
 * @file keymanager.cpp
 * @brief Implements key derivation helpers (Argon2 via libsodium or fallback).
 */

#include "keymanager.h"
#include <QCryptographicHash>
#include <QtGlobal>

#ifdef HAVE_LIBSODIUM
#include <sodium.h>
#endif

QByteArray KeyManager::deriveKeyFromPassphrase(const QString &passphrase, const QByteArray &salt, int keyLength)
{
#ifdef HAVE_LIBSODIUM
    if (keyLength <= 0) return {};
    // Ensure libsodium initialized
    if (sodium_init() < 0) {
        // fallback to stub
        qWarning("libsodium failed to initialize; falling back to SHA256 stub for key derivation");
        // fall through to stub implementation below
    } else {
        QByteArray out;
        out.resize(keyLength);
        int r = crypto_pwhash(reinterpret_cast<unsigned char*>(out.data()), (unsigned long long)keyLength,
                              passphrase.toUtf8().constData(), (unsigned long long)passphrase.toUtf8().size(),
                              reinterpret_cast<const unsigned char*>(salt.constData()),
                              crypto_pwhash_OPSLIMIT_MODERATE,
                              crypto_pwhash_MEMLIMIT_MODERATE,
                              crypto_pwhash_ALG_ARGON2ID13);
        if (r == 0) return out;
        qWarning("Argon2 derivation failed; falling back to SHA256 stub");
    }
#endif

    // Fallback deterministic stub: not secure. For production use Argon2 via libsodium.
    QByteArray data = passphrase.toUtf8() + QByteArray(":") + salt;
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    if (keyLength <= hash.size()) return hash.left(keyLength);

    QByteArray out = hash;
    // If longer key requested, repeat the hash (simple expand, not cryptographically strong)
    while (out.size() < keyLength) {
        hash = QCryptographicHash::hash(hash + data, QCryptographicHash::Sha256);
        out.append(hash);
    }
    return out.left(keyLength);
}
