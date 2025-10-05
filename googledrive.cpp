#include "googledrive.h"
#include "googleauthenticator.h"
#include "googlefiledownload.h"
#include <QTimer>
#include <QFile>
#include <QFileInfo>


GoogleDrive::GoogleDrive(GoogleFileList &fileList, GoogleAuthenticator &authenticator, QObject *parent)
    : QObject(parent),
    m_files(fileList),
    m_authenticator(authenticator),
    m_fileDiscoveryBusy(false),
    m_netManager()
{
    // Connect the network manager to our reply handler for the file list
    connect(&m_netManager, &QNetworkAccessManager::finished, this, &GoogleDrive::onFileListReplyFinished);
}

bool GoogleDrive::discoveringFiles() const
{
    return m_fileDiscoveryBusy;
}

void GoogleDrive::startFileDiscovery()
{
    if (!m_fileDiscoveryBusy)
    {
        m_fileDiscoveryBusy = true;
        Q_EMIT fileDiscoveryStarted();

        // Queue the next retrieval up into the event queue
        QTimer::singleShot(1, this, [this]() {
            retrieveFiles();
        });
    }
}

void GoogleDrive::stopFileDiscovery()
{
    if (m_fileDiscoveryBusy)
    {
        m_fileDiscoveryBusy = false;
        Q_EMIT fileDiscoveryStopped();
    }
}

GoogleFileDownload* GoogleDrive::download(GoogleFile &file)
{
    // Prevent downloading folders or Google native files
    if (file.type().startsWith("application/vnd.google-apps.")) {
        qDebug() << "Skipping download for native Google file:" << file.name();
        return nullptr;
    }

    // Open the local file for writing (and fail if we can't)
    GoogleFileDownload *localFile = new GoogleFileDownload();
    if (!localFile->file.open())
    {
        qCritical() << "Failed to open file for writing:" << localFile->file.fileName();
        delete localFile;
        return nullptr;
    }

    qDebug() << "Attempting to download file:" << file.name() << "to:" << localFile->file.fileName();

    // Prepare the authenticated download request
    // The '?alt=media' parameter is crucial for downloading the file content.
    QUrl downloadUrl(QString("https://www.googleapis.com/drive/v3/files/%1?alt=media").arg(file.id()));
    QNetworkRequest request(downloadUrl);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authenticator.token()).toUtf8());

    QNetworkReply *reply = m_netManager.get(request);

    // Set properties and connect streaming signals
    reply->setProperty("RequestType", "FileDownload");
    reply->setProperty("LocalFilePath", localFile->file.fileName());

    // Store the file handle in the reply to manage its lifecycle
    reply->setProperty("LocalFileHandle", QVariant::fromValue(localFile));

    connect(reply, &QNetworkReply::readyRead, this, &GoogleDrive::onFileDownloadReadyRead);
    connect(reply, &QNetworkReply::finished, this, &GoogleDrive::onFileDownloadFinished);

    return localFile;
}

void GoogleDrive::retrieveFiles(const QString &pageToken)
{
    qDebug() << "GoogleDrive client attempting to list files...";

    // The Drive API endpoint for listing files
    QUrl driveUrl("https://www.googleapis.com/drive/v3/files");
    QUrlQuery query;
    // Limit results to 10 for a simple test
    query.addQueryItem("pageSize", "1000");
    if (!pageToken.isEmpty())
        query.addQueryItem("pageToken", pageToken);
    query.addQueryItem("fields", "nextPageToken, files(id, name, mimeType, size, md5Checksum, parents)");
    query.addQueryItem("corpora", "user");
    // query.addQueryItem("includeItemsFromAllDrives", "true");
    // query.addQueryItem("supportsAllDrives", "true");
    query.addQueryItem("q", "trashed=false");
    driveUrl.setQuery(query);

    QNetworkRequest request(driveUrl);

    // IMPORTANT: Set the Authorization header with the Bearer token
    QString token = m_authenticator.token();
    if (token.isEmpty()) {
        qCritical() << "Access token is empty! Cannot list files.";
        return;
    }
    request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());

    // Send the GET request
    m_netManager.get(request);
}

void GoogleDrive::onFileListReplyFinished(QNetworkReply *reply)
{
    // Check if the reply is for the file list
    if (reply->url().toString().contains("drive/v3/files")) {
        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << "\n--- Drive API Request Failed ---";
            qCritical() << "Error:" << reply->errorString();
            qCritical() << "Response Body:" << reply->readAll();
            qCritical() << "------------------------------\n";
        } else {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject root = doc.object();

            qDebug() << "\n--- Drive File List Received Successfully ---";

            // Queue the next retrieval up into the event queue
            static int a = 0;
            a++;

            if (m_fileDiscoveryBusy && root.contains("nextPageToken"))
            {
                const QString nextPageToken = root["nextPageToken"].toString();
                QTimer::singleShot(1, this, [this, nextPageToken]() {
                    retrieveFiles(nextPageToken);
                });
            }
            else
            {
                qInfo() << "File discovery is complete.";
                stopFileDiscovery();
            }

            if (root.contains("files") && root["files"].isArray()) {
                QJsonArray files = root["files"].toArray();
                qDebug() << "Total files listed (up to 10):" << files.size();

                for (const QJsonValue &value : files) {
                    QJsonObject file = value.toObject();
                    QString parentId;
                    QStringList parents;
                    if (file.contains("parents") && file["parents"].isArray()) {
                        QJsonArray parentsArray = file["parents"].toArray();
                        if (!parentsArray.isEmpty()) {
                            // Files can have multiple parents; we use the first one for simplicity
                            parentId = parentsArray.first().toString();
                            for (const auto& element: parentsArray) {
                                parents.append(element.toVariant().toString());
                            }
                        }
                    }

                    const QString id = file["id"].toString();
                    const QString name = file["name"].toString();
                    const QString md5Sum = file["md5Checksum"].toString();
                    const QString mimeType = file["mimeType"].toString();

                    GoogleFile gf(id, name, md5Sum, parents, mimeType);
                    m_files.add(gf);

                    qDebug() << "  - Name:" << name
                             << "| ID:" << id
                             << "| Type:" << mimeType
                             << "| MD5Checksum:" << md5Sum
                             << "| Parent:" << parentId;
                }
            } else {
                qWarning() << "Response did not contain the 'files' array. Raw response:";
                qWarning() << responseData;
            }
        }
    }

    reply->deleteLater();
}

/**
     * @brief Slot called when new data is available during a file download.
     * This streams the incoming data chunk directly to the local file.
     */
void GoogleDrive::onFileDownloadReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    GoogleFileDownload *localFile = reply->property("LocalFileHandle").value<GoogleFileDownload*>();
    if (localFile)
    {
        // Handle file size
        if (localFile->fileSize() < 0)
        { // Check if we haven't recorded the size yet
            qint64 totalFileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();

            // Store the size for progress tracking
            localFile->setFileSize(totalFileSize);
        }

        // Store data
        if (localFile->file.isOpen())
        {
            QByteArray d = reply->readAll();
            localFile->file.write(d);
            localFile->increaseDownloaded(d.size());
        }
    }
}

/**
     * @brief Slot called when a file download is completed (successfully or with error).
     * This handles file closure and cleanup.
     */
void GoogleDrive::onFileDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    GoogleFileDownload *localFile = reply->property("LocalFileHandle").value<GoogleFileDownload*>();
    QString localPath = reply->property("LocalFilePath").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "\n--- File Download Failed ---";
        qCritical() << "Error:" << reply->errorString();
        qCritical() << "--------------------------\n";

        if (localFile)
        {
            localFile->file.close();
            Q_EMIT localFile->failed();
        }
    } else {
        qInfo() << "\n--- File Download Complete! ---";
        qInfo() << "File saved successfully to:" << localPath;
        qInfo() << "File size:" << QFileInfo(localPath).size() << "bytes";
        qInfo() << "-----------------------------\n";

        if (localFile)
        {
            Q_EMIT localFile->completed();
        }
    }

    reply->deleteLater();
}
