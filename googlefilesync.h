#pragma once

#include <QObject>
#include <QDir>
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
        OUTOFSYNC,
        SYNCING,
        SYNCED
    };
    State state() const;

    int progress() const;

    const GoogleFile& file() const;

    //! @brief Directory where the local file resides.
    QDir localFileDir() const;
    //! @brief Path of the local file.
    QString localFilePath() const;

    static QString toString(State s);

public Q_SLOTS:
    void analyze();
    void synchronize();

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
    const QDir m_localFileDir;
    const QString m_localFilePath;

    State m_state;
    int m_progress;

    GoogleFileDownload *m_pDownload;
    qint64 m_downloaded;
};
