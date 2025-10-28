#pragma once

#include <QMutex>
#include <QAtomicInt>
#include <QQueue>
#include <QWaitCondition>
#include "googlefilesync.h"


class GoogleSyncData
{
public:
    GoogleSyncData();

    QMutex mutex;
    bool m_keepRunning;
    QAtomicInt jobsCount;
    QWaitCondition waitCondition;
    //! @brief Files waiting for MD5 comparison
    QQueue<GoogleFileSync *> m_analysisQueue;
};
