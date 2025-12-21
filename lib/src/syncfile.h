#pragma once

#include <QObject>
#include <QSharedData>
#include "storageformat.h"
#include "syncfiledata.h"
class GoogleFile;


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

    QString fileId() const;
    QString fileName() const;
    QString md5Sum() const;
    QDateTime lastModified() const;

    StorageFormat storageFormat() const;

    static SyncFile fromFile(const QString &file);
    static SyncFile fromFile(const GoogleFile &file);

public Q_SLOTS:
    /**
     * @brief Store the SyncFile's data into a *.gsync file 
     * @param fromFile The source file path from which the data will be stored.
     * @return true if the restoration was successful, false otherwise.
     */
    bool store(const QString &fromFile);
    /** 
     * @brief Restore the SyncFile's data from a *.gsync file into a given file path.
     * @param toFile The destination file path where the data will be restored.
     * @return true if the restoration was successful, false otherwise.
     */
    bool restore(const QString &toFile);

signals:

private:
    QSharedDataPointer<SyncFileData> d;
};
