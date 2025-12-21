#pragma once

#include <QSharedData>
#include <QString>
#include <QDate>
#include "storageformat.h"


class SyncFileData: public QSharedData
{
public:
    SyncFileData() = default;
    // The copy constructor is needed for QSharedData, but can be defaulted.
    SyncFileData(const SyncFileData& other) = default;
    ~SyncFileData() = default;

    QString fileId;
    QString fileName;
    QString md5Sum;
    QDateTime lastModified;
    int size;
    StorageFormat storageFormat;
};
