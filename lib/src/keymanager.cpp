#include "keymanager.h"
#include <QCryptographicHash>

QByteArray KeyManager::deriveKeyFromPassphrase(const QString &passphrase, const QByteArray &salt, int keyLength)
{
    // Simple deterministic stub: not secure. For production use Argon2 via libsodium.
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
