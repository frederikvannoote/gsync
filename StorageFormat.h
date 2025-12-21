#pragma once

#include <QString>


enum class StorageFormat
{
    RAW
};

static QString toString(StorageFormat format)
{
    switch (format)
    {
    case StorageFormat::RAW:
    default:
        return "RAW";
    }
}
