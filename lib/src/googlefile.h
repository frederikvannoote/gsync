/**
 * @file googlefile.h
 * @brief Value object that represents a file metadata entry from Google Drive.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QSharedDataPointer>
#include <qdatetime.h>
class GoogleFilePrivate;


/**
 * @brief Public value class for a Google Drive File.
 *
 * This class uses implicit sharing to provide efficient copy and assignment operations.
 * It is suitable for use in Qt containers like QVector and QMap.
 */
class GoogleFile
{
public:
    // Constructors and Destructor
    GoogleFile();
    GoogleFile(const QString &id,
               const QString &name,
               const QString &md5sum,
               const QStringList &parents,
               const QString &type,
               const QDateTime &lastModified,
               int size);
    // Copy Constructor and Assignment Operator are provided by QSharedDataPointer
    // The PIMPL class is deep-copied only when modifications are made (Copy-on-Write).
    ~GoogleFile();
    GoogleFile(const GoogleFile& other);
    GoogleFile& operator=(const GoogleFile& other);

    /**
     * @brief Returns true if the object holds a valid file entry.
     */
    bool isValid() const;

    // --- Getters (Const access) ---
    /** @brief Remote id assigned by Google Drive */
    QString id() const;
    /** @brief User visible filename */
    QString name() const;
    /** @brief Hex-encoded MD5 checksum of the stored content */
    QString md5Sum() const;
    /** @brief List of parent folder ids */
    QStringList parents() const;
    /** @brief MIME type or inferred type string */
    QString type() const; // Maps to mimeType internally
    /** @brief Path computed from parent chain (if available) */
    QString path() const;
    /** @brief Last modification timestamp reported by Drive */
    QDateTime lastModified() const;
    /** @brief Size in bytes */
    int size() const;

    // --- Setters (Triggers Copy-on-Write if data is shared) ---
    /** @brief Set the remote id */
    void setId(const QString &newId);
    /** @brief Set the display name */
    void setName(const QString &newName);
    /** @brief Set the MD5 sum */
    void setMd5Sum(const QString &newMd5Sum);
    /** @brief Set parent folder ids */
    void setParents(const QStringList &newParents);
    /** @brief Set the type/mime-type */
    void setType(const QString &newType); // Maps to mimeType internally
    /** @brief Set the path */
    void setPath(const QString &path);

private:
    QSharedDataPointer<GoogleFilePrivate> d;
};
