/**
 * @file encryption.h
 * @brief High-level streaming encryption helpers used by the gsync library.
 *
 * The Encryption class provides a small convenience wrapper to encrypt and decrypt
 * streams using a secure AEAD cipher (XChaCha20-Poly1305) when libsodium is
 * available. The format is chunked and authenticated; callers should provide
 * a binary key (e.g., derived via Argon2).
 */

#pragma once

#include <QIODevice>
#include <QByteArray>
#include <QString>
#include <functional>

class Encryption
{
public:
    /**
     * @brief Parameters that control chunking and algorithm selection.
     */
    struct Params {
        /// Size of each plaintext chunk (default 64 KiB)
        int chunkSize = 64 * 1024; // default 64 KiB

        /// Human-readable algorithm hint (for metadata only)
        QString alg = "XChaCha20-Poly1305";

        /// Whether to use chunking; currently always true for the implementation
        bool chunked = true;
    };

    /**
     * @brief Returns true when a real crypto backend is available (e.g., libsodium).
     */
    static bool isAvailable();

    /**
     * @brief Encrypts data from `in` to `out` using the provided raw `key` (binary).
     *
     * Data is written in a secure, authenticated chunked format. The progress
     * callback receives (processedBytes, totalBytes) where totalBytes may be -1
     * if unknown (e.g., sequential streams).
     *
     * @param in Readable QIODevice providing plaintext.
     * @param out Writable QIODevice receiving encrypted data.
     * @param key Binary encryption key (length must satisfy the chosen cipher).
     * @param params Encryption parameters (chunk size, alg hints).
     * @param progress Optional progress callback.
     * @return true on success, false on error (including authentication failures).
     */
    static bool encryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress = nullptr);

    /**
     * @brief Decrypts data from `in` to `out` using the provided raw `key` (binary).
     *
     * The input must follow the format produced by `encryptStream`.
     *
     * @param in Readable QIODevice providing encrypted input.
     * @param out Writable QIODevice receiving decrypted plaintext.
     * @param key Binary decryption key.
     * @param params Params structure (only chunkSize is validated against header).
     * @param progress Optional progress callback.
     * @return true on success, false on error (including authentication failures and format errors).
     */
    static bool decryptStream(QIODevice &in, QIODevice &out, const QByteArray &key, const Params &params, std::function<void(qint64,qint64)> progress = nullptr);
};
