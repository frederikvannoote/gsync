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
    bool store(const QString &fromFile);
    // void restore();

signals:

private:
    QSharedDataPointer<SyncFileData> d;
};
