#include "googlefile.h"

/**
 * @brief The private data structure implementing the PIMPL idiom.
 * This class inherits QSharedData to enable implicit sharing (copy-on-write).
 * It holds the actual data members.
 */
class GoogleFilePrivate : public QSharedData
{
public:
    bool valid;
    // Fields retrieved from Google Drive API
    QString id;
    QString name;
    QString md5Sum;
    QStringList parents; // Array of parent folder IDs
    QString mimeType;    // Mime type (e.g., 'image/jpeg' or 'application/vnd.google-apps.folder')
    QString path;

    GoogleFilePrivate() = default;
    // The copy constructor is needed for QSharedData, but can be defaulted.
    GoogleFilePrivate(const GoogleFilePrivate& other) = default;
    ~GoogleFilePrivate() = default;
};

/**
 * @brief Default Constructor.
 * Allocates the private data object and initializes the shared pointer.
 */
GoogleFile::GoogleFile() : d(new GoogleFilePrivate)
{
    // The members of GoogleFilePrivate are default initialized (QString to "", qint64 to 0)
    d->valid = false;
}

GoogleFile::GoogleFile(const QString &id, const QString &name, const QString &md5sum, const QStringList &parents, const QString &type):
    d(new GoogleFilePrivate)
{
    d->valid = true;
    d->id = id;
    d->name = name;
    d->md5Sum = md5sum;
    d->parents = parents;
    d->mimeType = type;
}

/**
 * @brief Destructor.
 * QSharedDataPointer automatically handles reference counting and deletion of the private data.
 */
GoogleFile::~GoogleFile() = default;

/**
 * @brief Copy Constructor.
 * QSharedDataPointer implements shared copying by incrementing the reference count.
 */
GoogleFile::GoogleFile(const GoogleFile& other) : d(other.d)
{
}

/**
 * @brief Assignment Operator.
 * QSharedDataPointer implements shared assignment by replacing the pointer and handling ref counts.
 */
GoogleFile& GoogleFile::operator=(const GoogleFile& other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

bool GoogleFile::isValid() const
{
    return d->valid;
}

// --- Getters (Const methods - Read-only access to private data) ---

QString GoogleFile::id() const { return d->id; }
QString GoogleFile::name() const { return d->name; }
QString GoogleFile::md5Sum() const { return d->md5Sum; }
QStringList GoogleFile::parents() const { return d->parents; }
QString GoogleFile::type() const { return d->mimeType; }
QString GoogleFile::path() const { return d->path; }

// --- Setters (Modify access - Triggers Copy-on-Write) ---

void GoogleFile::setId(const QString &newId)
{
    // d.detach() ensures that if 'd' is currently shared, a deep copy is made before modification.
    d.detach();
    d->id = newId;
}

void GoogleFile::setName(const QString &newName)
{
    d.detach();
    d->name = newName;
}

void GoogleFile::setMd5Sum(const QString &newMd5Sum)
{
    d.detach();
    d->md5Sum = newMd5Sum;
}

void GoogleFile::setParents(const QStringList &newParents)
{
    d.detach();
    d->parents = newParents;
}

void GoogleFile::setType(const QString &newType)
{
    d.detach();
    d->mimeType = newType;
}

void GoogleFile::setPath(const QString &path)
{
    d.detach();
    d->path = path;
}
