/**
 * @file syncfile.h
 * @brief Represents a single file entry in a .gsync archive and provides store/restore helpers.
 *
 * The SyncFile class holds metadata about a file synchronized from Google Drive and
 * offers convenience methods to store and restore file contents to/from the custom
 * .gsync archive formats.
 */

#pragma once

#include <QObject>
#include <QSharedData>
#include "storageformat.h"
#include "syncfiledata.h"
class GoogleFile;


/**
 * @class SyncFile
 * @brief Represents metadata and operations for a synchronized file.
 *
 * SyncFile is a lightweight QObject wrapper around shared data that contains file
 * identity and metadata (MD5, modification time, size and storage format). It
 * exposes convenience methods to persist the underlying file data to a *.gsync
 * package and to restore it back to disk.
 */
class SyncFile : public QObject
{
    Q_OBJECT

public:
    SyncFile(const QString &fileId,
             const QString &fileName,
             const QString md5Sum,
             const QDateTime &lastModified,
             StorageFormat storageFormat,
             QObject *parent = nullptr);
    ~SyncFile();
    SyncFile(const SyncFile& other);
    SyncFile& operator=(const SyncFile& other);

    /**
     * @brief Returns the remote file id.
     */
    QString fileId() const;

    /**
     * @brief Returns the file name.
     */
    QString fileName() const;

    /**
     * @brief Returns the MD5 digest of the file contents as a hex string.
     */
    QString md5Sum() const;

    /**
     * @brief Returns the last modification timestamp reported by Drive.
     */
    QDateTime lastModified() const;

    /**
     * @brief Returns how the file is stored inside the .gsync package.
     */
    StorageFormat storageFormat() const;

    /**
     * @brief Create a SyncFile instance by reading a .gsync file from disk.
     * @param file Path to an existing .gsync file to parse.
     * @return A SyncFile populated from the archive header and metadata.
     */
    static SyncFile fromFile(const QString &file);

    /**
     * @brief Create a SyncFile instance from a GoogleFile value object.
     * @param file The GoogleFile value object.
     */
    static SyncFile fromFile(const GoogleFile &file);

public Q_SLOTS:
    /**
     * @brief Store the SyncFile's data into a *.gsync file
     * @param fromFile The source file path from which the data will be stored.
     * @return true on success, false on error.
     */
    bool store(const QString &fromFile);

    /**
     * @brief Restore the SyncFile's data from a *.gsync file into a given file path.
     * @param toFile The destination file path where the data will be restored.
     * @return true on success, false on error.
     */
    bool restore(const QString &toFile);

signals:

private:
    QSharedDataPointer<SyncFileData> d;
};
