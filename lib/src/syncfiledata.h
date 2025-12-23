/**
 * @file syncfiledata.h
 * @brief Internal shared data structure used by SyncFile.
 */

#pragma once

#include <QSharedData>
#include <QString>
#include <QDate>
#include "storageformat.h"


/**
 * @brief Internal shared data which backs SyncFile.
 *
 * This POD-like struct is implicitly shared and intended for internal use. Fields
 * are public for performance and convenience.
 */
class SyncFileData: public QSharedData
{
public:
    SyncFileData() = default;
    // The copy constructor is needed for QSharedData, but can be defaulted.
    SyncFileData(const SyncFileData& other) = default;
    ~SyncFileData() = default;

    /// Remote file identifier
    QString fileId;

    /// File name
    QString fileName;

    /// Hex-encoded MD5 sum
    QString md5Sum;

    /// Last modification timestamp
    QDateTime lastModified;

    /// File size in bytes
    int size;

    /// How the file data is formatted inside the package
    StorageFormat storageFormat;

    /// Path to the .gsync file (if loaded from or stored to a .gsync file)
    QString gsyncPath;
};
