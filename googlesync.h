#pragma once

#include <QObject>
#include <QQueue>
#include "googledrive.h"
#include "googlefile.h"
#include "googlefilesync.h"
class GoogleSyncData;


class GoogleSync : public QObject
{
    Q_OBJECT
public:
    explicit GoogleSync(GoogleDrive &drive, int maxRunningSyncs = 10, QObject *parent = nullptr);
    virtual ~GoogleSync();

    void setBaseDir(const QString &baseDirectory);
    QString baseDir() const;

    void add(GoogleFile &file);

    QVector<GoogleFileSync*> files() const;

    int runningSyncs() const;

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void started();
    void completed();

    void numberOfFilesChanged(int files);
    void runningSyncsChanged(int runningSyncs);
    void unknownItemsChanged(int unknown);
    void outOfSyncItemsChanged(int outOfSync);
    void syncedItemsChanged(int inSync);

private Q_SLOTS:
    void startNextSync();
    void onStateChanged();
    void onComplete();

private:
    GoogleDrive &m_drive;
    QString m_baseDir;

    QSharedPointer<GoogleSyncData> m_analysisData;

    //! @brief Files waiting for download/sync (after analysis)
    QQueue<GoogleFileSync *> m_syncQueue;

    int m_numberOfItems;
    int m_numberOfUnknownItems;
    int m_numberOfOutOfSyncItems;
    int m_numberOfSyncedItems;
    int m_runningSyncs;
    const int m_maxRunningSyncs;
};
