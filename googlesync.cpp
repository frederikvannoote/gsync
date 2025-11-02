#include "googlesync.h"
#include "googlesyncdata.h"
#include "fileanalyzerunnable.h"
#include <QTimer>
#include <QThreadPool>


GoogleSync::GoogleSync(GoogleDrive &drive, int maxRunningSyncs, QObject *parent)
    : QObject{parent}
    , m_drive(drive)
    , m_baseDir()
    , m_analysisData(new GoogleSyncData())
    , m_syncQueue()
    , m_numberOfItems(0)
    , m_numberOfUnknownItems(0)
    , m_numberOfOutOfSyncItems(0)
    , m_numberOfSyncedItems(0)
    , m_runningSyncs(0)
    , m_maxRunningSyncs(maxRunningSyncs)
{
    m_analysisData->m_keepRunning = true;

    // Start a bunch of analyzers. They will analyze the file asynchronously.
    QThreadPool *pool = QThreadPool::globalInstance();
    for (int i = 0; i < pool->maxThreadCount(); ++i)
        pool->start(new FileAnalyzeRunnable(m_analysisData));
}

GoogleSync::~GoogleSync()
{
    m_syncQueue.clear();

    {
        QMutexLocker<QMutex> locker(&m_analysisData->mutex);
        m_analysisData->m_keepRunning = false;
    }

    m_analysisData->waitCondition.wakeAll();

    QThreadPool::globalInstance()->waitForDone();
}

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
    connect(fs, &GoogleFileSync::stateChanged, this, &GoogleSync::onStateChanged);

    // Add to analyze queue (not protected by mutex)
    m_analysisData->m_analysisQueue.enqueue(fs);

    // Add to synchronize queue
    m_syncQueue.enqueue(fs);
}

QVector<GoogleFileSync *> GoogleSync::files() const
{
    return m_syncQueue.toVector();
}

int GoogleSync::runningSyncs() const
{
    return m_runningSyncs;
}

void GoogleSync::start()
{
    m_numberOfItems = m_syncQueue.count();
    Q_EMIT numberOfFilesChanged(m_numberOfItems);
    Q_EMIT started();

    // Start analyzers (protected by mutex)
    {
        QMutexLocker<QMutex> locker(&m_analysisData->mutex);
        QThreadPool *pool = QThreadPool::globalInstance();
        {
            for (int i = 0; i < pool->maxThreadCount(); ++i)
                m_analysisData->waitCondition.wakeOne();
        }
    }

    startNextSync();
}

void GoogleSync::startNextSync()
{
    while (m_runningSyncs < m_maxRunningSyncs && !m_syncQueue.isEmpty())
    {
        GoogleFileSync *fs = m_syncQueue.dequeue();
        while (fs->state() == GoogleFileSync::State::SYNCED)
            fs = m_syncQueue.dequeue();

        connect(fs, &GoogleFileSync::completed, this, &GoogleSync::onComplete);

        fs->synchronize();

        m_runningSyncs++;
        Q_EMIT runningSyncsChanged(m_runningSyncs);
    }

    if (m_syncQueue.isEmpty())
        Q_EMIT completed();
}

void GoogleSync::onStateChanged()
{
    int numberOfUnknownItems = 0;
    int numberOfOutOfSyncItems = 0;
    int numberOfSyncedItems = 0;

    for (GoogleFileSync *fs : m_syncQueue) {
        switch (fs->state())
        {
        case GoogleFileSync::State::UNKNOWN:
            numberOfUnknownItems++;
            break;
        case GoogleFileSync::State::SYNCED:
            numberOfSyncedItems++;
            break;
        case GoogleFileSync::State::OUTOFSYNC:
        default:
            numberOfOutOfSyncItems++;
            break;
        }
    }

    numberOfSyncedItems = m_numberOfItems - (m_syncQueue.count() - numberOfSyncedItems);

    if (numberOfUnknownItems != m_numberOfUnknownItems)
    {
        m_numberOfUnknownItems = numberOfUnknownItems;
        Q_EMIT unknownItemsChanged(numberOfUnknownItems);
    }
    if (numberOfOutOfSyncItems != m_numberOfOutOfSyncItems)
    {
        m_numberOfOutOfSyncItems = numberOfOutOfSyncItems;
        Q_EMIT outOfSyncItemsChanged(numberOfOutOfSyncItems);
    }
    if (numberOfSyncedItems != m_numberOfSyncedItems)
    {
        m_numberOfSyncedItems = numberOfSyncedItems;
        Q_EMIT syncedItemsChanged(numberOfSyncedItems);
    }
}

void GoogleSync::onComplete()
{
    m_runningSyncs--;
    Q_EMIT runningSyncsChanged(m_runningSyncs);

    if (!m_syncQueue.isEmpty())
        QTimer::singleShot(1, this, &GoogleSync::startNextSync);
}
