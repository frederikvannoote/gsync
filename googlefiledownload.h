#pragma once

#include <QObject>
#include <QTemporaryFile>


class GoogleFileDownload : public QObject
{
    Q_OBJECT
public:
    explicit GoogleFileDownload(QObject *parent = nullptr);

    QTemporaryFile file;

    void increaseDownloaded(qint64 downloaded);

    void setFileSize(qint64 size);
    qint64 fileSize() const;

Q_SIGNALS:
    void completed();
    void failed();
    void progress(qint64 downloaded);

private:
    qint64 m_downloaded;
    qint64 m_totalSize;
};
