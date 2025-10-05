#pragma once

#include <QString>
#include <QStringList>
#include <QSharedDataPointer>
class GoogleFilePrivate;


/**
 * @brief Public value class for a Google Drive File.
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
               const QString &type);
    // Copy Constructor and Assignment Operator are provided by QSharedDataPointer
    // The PIMPL class is deep-copied only when modifications are made (Copy-on-Write).
    ~GoogleFile();
    GoogleFile(const GoogleFile& other);
    GoogleFile& operator=(const GoogleFile& other);

    bool isValid() const;

    // --- Getters (Const access) ---
    QString id() const;
    QString name() const;
    QString md5Sum() const;
    QStringList parents() const;
    QString type() const; // Maps to mimeType internally
    QString path() const;

    // --- Setters (Triggers Copy-on-Write if data is shared) ---
    void setId(const QString &newId);
    void setName(const QString &newName);
    void setMd5Sum(const QString &newMd5Sum);
    void setParents(const QStringList &newParents);
    void setType(const QString &newType); // Maps to mimeType internally
    void setPath(const QString &path);

private:
    QSharedDataPointer<GoogleFilePrivate> d;
};
