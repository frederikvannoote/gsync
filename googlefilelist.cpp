#include "googlefilelist.h"

GoogleFileList::GoogleFileList(QObject *parent)
    : QObject{parent}
    , m_files()
{}

void GoogleFileList::add(GoogleFile &file)
{
    m_files[file.id()] = file;
    Q_EMIT added(file);
    Q_EMIT countChanged(count());
}

void GoogleFileList::remove(GoogleFile &file)
{
    size_t count = m_files.remove(file.id());
    if (count > 0)
    {
        Q_EMIT removed(file);
        Q_EMIT countChanged(this->count());
    }
}

GoogleFile GoogleFileList::file(const QString &id) const
{
    return m_files.value(id);
}

QVector<GoogleFile> GoogleFileList::files() const
{
    return m_files.values();
}

int GoogleFileList::count() const
{
    return m_files.count();
}
