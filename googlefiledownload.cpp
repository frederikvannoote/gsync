#include "googlefiledownload.h"

GoogleFileDownload::GoogleFileDownload(QObject *parent)
    : QObject{parent}
    , file()
    , m_downloaded(0)
    , m_totalSize(-1)
{}

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
