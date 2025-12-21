#include "syncfile.h"
#include "googlefile.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>


SyncFile::SyncFile(const QString &fileId,
         const QString &fileName,
         const QString md5Sum,
         const QDateTime &lastModified,
         StorageFormat storageFormat,
         QObject *parent)
    : QObject(parent)
    , d(new SyncFileData())
{
    d->fileId = fileId;
    d->fileName = fileName;
    d->md5Sum= md5Sum;
    d->lastModified = lastModified;
    d->storageFormat = storageFormat;
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
                    StorageFormat::RAW);
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
