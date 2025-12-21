#include "encryption.h"
#include <QDebug>

bool Encryption::isAvailable()
{
    return false;
}

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
