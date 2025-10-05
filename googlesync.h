#pragma once

#include <QObject>
#include <QQueue>
#include "googledrive.h"
#include "googlefile.h"
#include "googlefilesync.h"


class GoogleSync : public QObject
{
    Q_OBJECT
public:
    explicit GoogleSync(GoogleDrive &drive, int maxRunningSyncs = 10, QObject *parent = nullptr);

    void setBaseDir(const QString &baseDirectory);
    QString baseDir() const;

    void add(GoogleFile &file);

    QVector<GoogleFileSync*> files() const;

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void started();
    void completed();

private Q_SLOTS:
    void startNextSync();
    void onComplete();

private:
    GoogleDrive &m_drive;
    QString m_baseDir;
    QQueue<GoogleFileSync*> m_files;
    int m_runningSyncs;
    const int m_maxRunningSyncs;
};
