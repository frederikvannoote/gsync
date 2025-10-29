#include "googlefiledownload.h"

GoogleFileDownload::GoogleFileDownload(const QString &id, QObject *parent)
    : QObject{parent}
    , file()
    , m_id(id)
    , m_downloaded(0)
    , m_totalSize(-1)
    , m_retry(0)
{}

QString GoogleFileDownload::id() const
{
    return m_id;
}

void GoogleFileDownload::increaseDownloaded(qint64 downloaded)
{
    m_downloaded += downloaded;
    Q_EMIT progress(m_downloaded);
}

void GoogleFileDownload::setFileSize(qint64 size)
{
    m_totalSize = size;
    Q_EMIT progress(m_downloaded);
}

qint64 GoogleFileDownload::fileSize() const
{
    return m_totalSize;
}

void GoogleFileDownload::increaseRetry()
{
    m_downloaded = 0;
    m_retry++;
}

int GoogleFileDownload::retry() const
{
    return m_retry;
}
