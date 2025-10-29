#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QTemporaryFile>
#include "googlefilelist.h"
class GoogleAuthenticator;
class GoogleFileDownload;


/**
 * @brief Handles all interactions with the Google Drive API.
 */
class GoogleDrive : public QObject
{
    Q_OBJECT

public:
    explicit GoogleDrive(GoogleFileList &fileList,
                         GoogleAuthenticator &authenticator,
                         QObject *parent = nullptr);

    bool discoveringFiles() const;

public Q_SLOTS:
    void startFileDiscovery();
    void stopFileDiscovery();

    GoogleFileDownload *download(GoogleFile &file);

Q_SIGNALS:
    void fileDiscoveryStarted();
    void fileDiscoveryStopped();

private slots:
    //! @brief Sends an authenticated GET request to the Google Drive API to list files.
    void retrieveFiles(const QString &pageToken = QString());
    //! @brief Slot to handle the response from the Drive API.
    void onFileListReplyFinished(QNetworkReply *reply);
    //! @brief Slot called when new data is available during a file download.
    void onFileDownloadReadyRead();
    //! @brief Slot called when a file download is completed (successfully or with error).
    void onFileDownloadFinished();

private:
    void performDownload(GoogleFileDownload *localFile);

    GoogleFileList &m_files;
    GoogleAuthenticator &m_authenticator;
    bool m_fileDiscoveryBusy;
    QNetworkAccessManager m_netManager;
};
