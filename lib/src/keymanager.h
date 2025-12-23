/**
 * @file keymanager.h
 * @brief Helper functions for deriving cryptographic keys from passphrases.
 */

#pragma once

#include <QByteArray>
#include <QString>

class KeyManager
{
public:
    /**
     * @brief Derive a cryptographic key from a passphrase and salt.
     *
     * When libsodium is available, Argon2id is used; otherwise a deterministic
     * SHA-256-based fallback is used for tests and non-production builds.
     *
     * @param passphrase User-supplied passphrase.
     * @param salt Binary salt; should be unique per file.
     * @param keyLength Desired key length in bytes (e.g., 32 for AES-256/XChaCha20 keys).
     * @return Binary key of length `keyLength` or empty on error.
     */
    static QByteArray deriveKeyFromPassphrase(const QString &passphrase, const QByteArray &salt, int keyLength = 32);
};
