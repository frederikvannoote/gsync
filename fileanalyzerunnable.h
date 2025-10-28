#pragma once

#include <QObject>
#include <QRunnable>
#include <QSharedPointer>
class GoogleFileSync;
class GoogleSyncData;


/**
 * @brief QRunnable implementation to perform a non-blocking MD5 hash calculation.
 *
 * This task runs in a separate thread provided by QThreadPool, ensuring the
 * main application thread remains responsive during file analysis.
 */
class FileAnalyzeRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    FileAnalyzeRunnable(QSharedPointer<GoogleSyncData> data, QObject *parent = nullptr);

    //! @brief The entry point for the thread pool execution.
    void run() override;

signals:
    /**
     * @brief Signal emitted when the MD5 comparison is finished.
     * @param syncItem The GoogleFileSync item that was analyzed.
     * @param needsSync True if the local MD5 differs from the Google Drive MD5.
     * * NOTE: This signal must be connected to a slot in the main thread (Qt::QueuedConnection)
     * because it is emitted from a worker thread.
     */
    void analysisCompleted(GoogleFileSync *syncItem, bool needsSync);

private:
    QSharedPointer<GoogleSyncData> m_data;
};
