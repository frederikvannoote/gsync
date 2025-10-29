#pragma once

#include <QObject>
#include <QTemporaryFile>


class GoogleFileDownload : public QObject
{
    Q_OBJECT
public:
    explicit GoogleFileDownload(const QString &id, QObject *parent = nullptr);

    QString id() const;

    QTemporaryFile file;

    void increaseDownloaded(qint64 downloaded);

    void setFileSize(qint64 size);
    qint64 fileSize() const;

    void increaseRetry();
    int retry() const;

Q_SIGNALS:
    void completed();
    void failed();
    void progress(qint64 downloaded);

private:
    const QString m_id;
    qint64 m_downloaded;
    qint64 m_totalSize;
    int m_retry;
};
