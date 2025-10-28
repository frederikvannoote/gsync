#include "googlefilesync.h"
#include "googlefiledownload.h"
#include <QFile>
#include <QDebug>


GoogleFileSync::GoogleFileSync(GoogleDrive &drive,
                               GoogleFile &file,
                               const QString &baseDir,
                               QObject *parent)
    : QObject{parent}
    , m_drive(drive)
    , m_file(file)
    , m_baseDir(baseDir)
    , m_localFileDir(QDir(m_baseDir).filePath(m_file.path()))
    , m_localFilePath(m_localFileDir.filePath(m_file.name()))
    , m_state(UNKNOWN)
    , m_progress(0)
    , m_pDownload(nullptr)
    , m_downloaded(0)
{}

int GoogleFileSync::progress() const
{
    return m_progress;
}

const GoogleFile &GoogleFileSync::file() const
{
    return m_file;
}

QDir GoogleFileSync::localFileDir() const
{
    return m_localFileDir;
}

QString GoogleFileSync::localFilePath() const
{
    return m_localFilePath;
}

QString GoogleFileSync::toString(State s)
{
    switch (s)
    {
    case OUTOFSYNC:
        return "Out of sync";
    case SYNCING:
        return "Syncing";
    case SYNCED:
        return "In sync";
    case UNKNOWN:
    default:
        return "Unknown";
    }
}

void GoogleFileSync::analyze()
{
    const QString localFilePath = this->localFilePath();

    if (m_file.md5Sum() != calculateMd5Sum(localFilePath))
    {
        // Files are different
        qInfo() << localFilePath << "is out of sync to the file on Google Drive. It does need to be synced.";

        m_state = State::OUTOFSYNC;
        Q_EMIT stateChanged(m_state);
    }
    else
    {
        // Files are identical
        qInfo() << localFilePath << "is identical to the file on Google Drive. It does not need to be synced.";

        m_progress = 100;
        m_state = State::SYNCED;
        Q_EMIT stateChanged(m_state);
        Q_EMIT completed();
    }
}

void GoogleFileSync::synchronize()
{
    if (m_state == State::UNKNOWN)
        analyze();

    if (m_state == State::OUTOFSYNC)
    {
        m_downloaded = 0;
        m_pDownload = m_drive.download(m_file);
        connect(m_pDownload, &GoogleFileDownload::completed, this, &GoogleFileSync::onDownloadComplete);
        connect(m_pDownload, &GoogleFileDownload::progress, this, &GoogleFileSync::onProgressChanged);
        // connect(m_pDownload, &GoogleFileDownload::failed, ...);

        m_state = State::SYNCING;
        Q_EMIT stateChanged(m_state);
        Q_EMIT started();
    }
}

void GoogleFileSync::onDownloadComplete()
{
    const QString localFileDir = this->localFileDir().path();
    const QString localFilePath = this->localFilePath();
    qInfo() << "Move file to" << localFilePath;

    QDir::root().mkpath(localFileDir);
    QFile::copy(m_pDownload->file.fileName(), localFilePath);
    delete m_pDownload;
    m_pDownload = nullptr;

    m_state = State::SYNCED;
    Q_EMIT stateChanged(m_state);
    Q_EMIT completed();
}

void GoogleFileSync::onProgressChanged(qint64 value)
{
    qint64 max = 0;
    if (m_pDownload)
        max = m_pDownload->fileSize();

    if (max > 0)
    {
        m_progress = ceil((value*100) / max);

        Q_EMIT progressChanged(m_progress);
    }
}

GoogleFileSync::State GoogleFileSync::state() const
{
    return m_state;
}

/**
 * @brief Utility function to calculate the MD5 hash of a local file.
 * @param filePath The path to the file.
 * @return The MD5 checksum as a hex string, or an empty string on error.
 */
QString GoogleFileSync::calculateMd5Sum(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open file for MD5 calculation:" << filePath;
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Md5);

    // Read the file in chunks to handle large files efficiently without loading the whole file into memory.
    const int chunkSize = 1024 * 1024; // 1MB chunks
    while (!file.atEnd()) {
        hash.addData(file.read(chunkSize));
    }

    file.close();

    // Return the MD5 hash as a hexadecimal string
    return hash.result().toHex();
}
