/**
 * @file syncfile.cpp
 * @brief Implementation of SyncFile helpers: parsing, store and restore of .gsync archives.
 */

#include "syncfile.h"
#include "googlefile.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QCryptographicHash>


SyncFile::SyncFile(const QString &fileId,
         const QString &fileName,
         const QString md5Sum,
         const QDateTime &lastModified,
         StorageFormat storageFormat,
         const QByteArray &keyDerivationSalt,
         const Encryption::Params &encryptionParams,
         QObject *parent)
    : QObject(parent)
    , d(new SyncFileData())
{
    d->fileId = fileId;
    d->fileName = fileName;
    d->md5Sum= md5Sum;
    d->lastModified = lastModified;
    d->storageFormat = storageFormat;
    d->keyDerivationSalt = keyDerivationSalt;
    d->encryptionParams = encryptionParams;
}

SyncFile::~SyncFile() = default;

/**
 * @brief Copy Constructor.
 * QSharedDataPointer implements shared copying by incrementing the reference count.
 */
SyncFile::SyncFile(const SyncFile& other) : d(other.d)
{
}

/**
 * @brief Assignment Operator.
 * QSharedDataPointer implements shared assignment by replacing the pointer and handling ref counts.
 */
SyncFile& SyncFile::operator=(const SyncFile& other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

QString SyncFile::fileId() const
{
    return d->fileId;
}

QString SyncFile::fileName() const
{
    return d->fileName;
}

QString SyncFile::md5Sum() const
{
    return d->md5Sum;
}

QDateTime SyncFile::lastModified() const
{
    return d->lastModified;
}

StorageFormat SyncFile::storageFormat() const
{
    return d->storageFormat;
}

SyncFile SyncFile::fromFile(const GoogleFile &file)
{
    return SyncFile(file.id(),
                    file.name(),
                    file.md5Sum(),
                    file.lastModified(),
                    StorageFormat::RAW, // Default to RAW for now, until encryption options are available in the UI
                    {},
                    {});
}

SyncFile SyncFile::fromFile(const QString &file)
{
    QFile source(file);
    if (!source.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open .gsync file:" << source.errorString();
        return SyncFile(QString(), QString(), QString(), QDateTime(), StorageFormat::RAW);
    }

    const QByteArray separator = "---END-OF-HEADER---\r\n";
    QByteArray buffer;
    QByteArray headerBytes;
    const qint64 chunkSize = 8192;
    while (!source.atEnd()) {
        buffer = source.read(chunkSize);
        headerBytes.append(buffer);
        int idx = headerBytes.indexOf(separator);
        if (idx != -1) {
            QByteArray jsonBytes = headerBytes.left(idx);
            QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
            if (!doc.isObject()) {
                qWarning() << "Invalid header JSON in .gsync file";
                source.close();
                return SyncFile(QString(), QString(), QString(), QDateTime(), StorageFormat::RAW);
            }
            QJsonObject obj = doc.object();
            QString id = obj.value("id").toString();
            QString name = obj.value("name").toString();
            QString md5 = obj.value("md5sum").toString();
            QDateTime lastModified = QDateTime::fromString(obj.value("lastModified").toString(), Qt::ISODate);
            QString sf = obj.value("storageFormat").toString();
            StorageFormat format = StorageFormat::RAW;
            if (sf == "RAW") {
                format = StorageFormat::RAW;
            } else if (sf == "ENCRYPTED") {
                format = StorageFormat::ENCRYPTED;
            }

            QByteArray keyDerivationSalt;
            Encryption::Params encryptionParams;

            if (format == StorageFormat::ENCRYPTED) {
                keyDerivationSalt = QByteArray::fromHex(obj.value("keyDerivationSalt").toString().toUtf8());
                if (obj.contains("encryptionParams")) {
                    QJsonObject encParamsObj = obj.value("encryptionParams").toObject();
                    encryptionParams.chunkSize = encParamsObj.value("chunkSize").toInt(encryptionParams.chunkSize);
                    encryptionParams.alg = encParamsObj.value("alg").toString(encryptionParams.alg);
                    encryptionParams.chunked = encParamsObj.value("chunked").toBool(encryptionParams.chunked);
                }
            }

            SyncFile s(id, name, md5, lastModified, format, keyDerivationSalt, encryptionParams);
            s.d->gsyncPath = file;
            QFileInfo fi(file);
            s.d->size = static_cast<int>(fi.size());
            source.close();
            return s;
        }
        if (headerBytes.size() > 1024 * 1024) {
            qWarning() << "Header too large in .gsync file";
            break;
        }
    }
    source.close();
    return SyncFile(QString(), QString(), QString(), QDateTime(), StorageFormat::RAW);
}

bool SyncFile::restore(const QString &toFile)
{
    if (d->gsyncPath.isEmpty()) {
        qWarning() << "No .gsync source path available for restore";
        return false;
    }

    QFile source(d->gsyncPath);
    if (!source.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open .gsync source file:" << source.errorString();
        return false;
    }

    const QByteArray separator = "---END-OF-HEADER---\r\n";
    QByteArray buffer;
    QByteArray headerBytes;
    const qint64 chunkSize = 8192;
    qint64 headerEnd = -1;

    while (!source.atEnd()) {
        buffer = source.read(chunkSize);
        headerBytes.append(buffer);
        headerEnd = headerBytes.indexOf(separator);
        if (headerEnd != -1) {
            break;
        }
        if (headerBytes.size() > 1024 * 1024) {
            qWarning() << "Header too large in .gsync file";
            source.close();
            return false;
        }
    }

    if (headerEnd == -1) {
        qWarning() << "Missing header separator in .gsync file";
        source.close();
        return false;
    }

    qint64 bodyStart = headerEnd + separator.size();

    QFile destination(toFile);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not open destination file for restore:" << destination.errorString();
        source.close();
        return false;
    }

    // Write any already read bytes that belong to the body
    QByteArray bodyChunk = headerBytes.mid(bodyStart);
    if (!bodyChunk.isEmpty()) {
        qint64 written = destination.write(bodyChunk);
        if (written != bodyChunk.size()) {
            qWarning() << "Failed to write initial body chunk during restore";
            source.close();
            destination.close();
            return false;
        }
    }

    // Continue streaming the remaining data
    while (!source.atEnd()) {
        buffer = source.read(chunkSize);
        if (buffer.isEmpty()) break;
        qint64 written = destination.write(buffer);
        if (written != buffer.size()) {
            qWarning() << "Error writing to destination during restore";
            source.close();
            destination.close();
            return false;
        }
    }

    source.close();
    destination.close();

    // Verify MD5 if provided
    if (!d->md5Sum.isEmpty()) {
        QFile verify(toFile);
        if (verify.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            while (!verify.atEnd()) {
                buffer = verify.read(chunkSize);
                hash.addData(buffer);
            }
            verify.close();
            QByteArray computed = hash.result().toHex();
            if (computed != d->md5Sum.toUtf8()) {
                qWarning() << "MD5 mismatch after restore" << computed << d->md5Sum.toUtf8();
                return false;
            }
        }
    }

    qDebug() << "Restore completed for" << toFile;
    return true;
}

bool SyncFile::store(const QString &fromFile)
{
    // 1. Open the source file for reading
    QFile sourceFile(fromFile);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open source file:" << sourceFile.errorString();
        return false;
    }

    // 2. Open the destination file for writing (and truncate if it exists)
    QFile destinationFile(fromFile + ".gsync");
    if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not open destination file:" << destinationFile.errorString();
        sourceFile.close();
        return false;
    }

    // Write the header
    QJsonObject header;
    header["id"] = d->fileId;
    header["name"] = d->fileName;
    header["md5sum"] = d->md5Sum;
    header["lastModified"] = d->lastModified.toString(Qt::ISODate);
    header["storageFormat"] = toString(d->storageFormat);
    QJsonDocument d(header);
    destinationFile.write(d.toJson(QJsonDocument::JsonFormat::Indented));
    destinationFile.write("---END-OF-HEADER---\r\n");

    // Define the buffer size (e.g., 8 KB)
    const int bufferSize = 8192;
    QByteArray buffer;

    // 3. Loop: Read chunk by chunk and write to the destination
    while (!sourceFile.atEnd()) {
        // Read data into the QByteArray buffer (up to bufferSize)
        buffer = sourceFile.read(bufferSize);

        // Write the read data to the destination file
        qint64 bytesWritten = destinationFile.write(buffer);

        // Crucial: Check if a write error occurred
        if (bytesWritten == -1) {
            qWarning() << "Error writing to destination file:" << destinationFile.errorString();
            sourceFile.close();
            destinationFile.close();
            return false;
        }

        // Check if all bytes read were successfully written
        if (bytesWritten != buffer.size()) {
            qWarning() << "Not all bytes could be written (Possible disk space issue).";
            sourceFile.close();
            destinationFile.close();
            return false;
        }
    }

    // 4. Close files and return success
    sourceFile.close();
    destinationFile.close();
    qDebug() << "File successfully copied in chunks.";
    return true;
}
