#pragma once

#include <QObject>
#include "googledrive.h"
#include "googlefile.h"
class GoogleFileDownload;


class GoogleFileSync : public QObject
{
    Q_OBJECT
public:
    GoogleFileSync(GoogleDrive &drive,
                   GoogleFile &file,
                   const QString &baseDir,
                   QObject *parent = nullptr);

    enum State
    {
        UNKNOWN,
        SYNCING,
        SYNCED
    };
    State state() const;

    int progress() const;

    const GoogleFile& file() const;

    static QString toString(State s);

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void started();
    void completed();
    void progressChanged(int value);
    void stateChanged(const State &state);

private Q_SLOTS:
    void onDownloadComplete();
    void onProgressChanged(qint64 value);

private:
    static QString calculateMd5Sum(const QString &filePath);

    GoogleDrive &m_drive;
    GoogleFile m_file;
    const QString m_baseDir;

    State m_state;
    int m_progress;

    GoogleFileDownload *m_pDownload;
    qint64 m_downloaded;
};
