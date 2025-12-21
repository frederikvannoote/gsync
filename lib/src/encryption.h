#pragma once

#include <QIODevice>
#include <QByteArray>
#include <QString>
#include <functional>

class Encryption
{
public:
    struct Params {
        int chunkSize = 64 * 1024; // default 64 KiB
        QString alg = "XChaCha20-Poly1305";
        bool chunked = true;
    };

    // Returns true when a real crypto backend is available (e.g., libsodium)
    static bool isAvailable();

    // Encrypts data from `in` to `out` using the provided raw `key` (binary). Progress callback receives (processedBytes, totalBytes) where totalBytes may be -1 if unknown.
    static bool encryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress = nullptr);

    // Decrypts data from `in` to `out` using the provided raw `key` (binary).
    static bool decryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress = nullptr);
};
