/**
 * @file storageformat.h
 * @brief Enumerates supported on-disk storage formats for file data in the .gsync package.
 */

#pragma once

#include <QString>


/**
 * @enum StorageFormat
 * @brief Supported storage formats for archived file contents.
 */
enum class StorageFormat
{
    /// Raw (uncompressed, unencrypted) file contents
    RAW
};

/**
 * @brief Convert a StorageFormat to a human-readable string.
 */
static QString toString(StorageFormat format)
{
    switch (format)
    {
    case StorageFormat::RAW:
    default:
        return "RAW";
    }
}
