#include "fileanalyzerunnable.h"
#include "googlefilesync.h"
#include "googlesyncdata.h"
#include <QMutexLocker>


FileAnalyzeRunnable::FileAnalyzeRunnable(QSharedPointer<GoogleSyncData> data,
                                         QObject *parent)
    : QObject{parent}
    , m_data(data)
{
}

void FileAnalyzeRunnable::run()
{
    while (m_data->m_keepRunning)
    {
        // Wait for new file to analyze.
        GoogleFileSync *file = nullptr;
        {
            QMutexLocker locker(&m_data->mutex);
            if (m_data->m_analysisQueue.isEmpty())
            {
                m_data->waitCondition.wait(&m_data->mutex); // Wait for new work
                {
                    if (!m_data->m_keepRunning)
                    {
                        break; // No more work, exit thread
                    }
                }
            }

            {
                file = m_data->m_analysisQueue.dequeue();
            }
        }

        // Analyze the file.
        file->analyze();
    }
}
