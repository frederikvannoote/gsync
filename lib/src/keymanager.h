#pragma once

#include <QByteArray>
#include <QString>

class KeyManager
{
public:
    // Derive a key from a passphrase + salt. Returns a binary key.
    // NOTE: the stub implementation uses SHA-256 for determinism and is NOT recommended for production.
    static QByteArray deriveKeyFromPassphrase(const QString &passphrase, const QByteArray &salt, int keyLength = 32);
};
