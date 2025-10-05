#include "googlesync.h"
#include <QTimer>


GoogleSync::GoogleSync(GoogleDrive &drive, int maxRunningSyncs, QObject *parent)
    : QObject{parent}
    , m_drive(drive)
    , m_baseDir()
    , m_files()
    , m_runningSyncs(0)
    , m_maxRunningSyncs(maxRunningSyncs)
{}

void GoogleSync::setBaseDir(const QString &baseDirectory)
{
    m_baseDir = baseDirectory;
}

QString GoogleSync::baseDir() const
{
    return m_baseDir;
}

void GoogleSync::add(GoogleFile &file)
{
    GoogleFileSync *fs = new GoogleFileSync(m_drive, file, m_baseDir, this);
    m_files.enqueue(fs);
}

QVector<GoogleFileSync *> GoogleSync::files() const
{
    return m_files.toVector();
}

void GoogleSync::start()
{
    Q_EMIT started();
    startNextSync();
}

void GoogleSync::startNextSync()
{
    while (m_runningSyncs < m_maxRunningSyncs && !m_files.isEmpty())
    {
        GoogleFileSync *fs = m_files.dequeue();
        connect(fs, &GoogleFileSync::completed, this, &GoogleSync::onComplete);

        fs->start();

        m_runningSyncs++;
    }

    if (m_files.isEmpty())
        Q_EMIT completed();
}

void GoogleSync::onComplete()
{
    m_runningSyncs--;
    QTimer::singleShot(1, this, &GoogleSync::startNextSync);
}
