#include "googleauthenticator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>

#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

// Scope for Google Drive: read and write to files created or opened by this app.
// Use "https://www.googleapis.com/auth/drive" for full access.
const QString SCOPE = "https://www.googleapis.com/auth/drive"; //"https://www.googleapis.com/auth/drive.file";

// The port for the local HTTP server to listen on.
// This MUST match the port in the Redirect URI configured in the Google Cloud Console:
// "http://127.0.0.1:8080/cb"
const quint16 LISTEN_PORT = 8080;
// --- End Configuration ---


GoogleAuthenticator::GoogleAuthenticator(const QString &client_id,
                                         const QString &client_secret,
                                         QObject *parent):
    QObject(parent),
    m_oauth(),
    m_refreshTimer()
{
    connect(&m_refreshTimer, &QTimer::timeout, &m_oauth, &QOAuth2AuthorizationCodeFlow::refreshAccessToken);

    // 2. Set Google's OAuth endpoints
    m_oauth.setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
    m_oauth.setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));

    // 3. Set the application credentials
    m_oauth.setClientIdentifier(client_id);
    m_oauth.setClientIdentifierSharedKey(client_secret);
    m_oauth.setScope(SCOPE);

    // 4. Set the reply handler: starts a local HTTP server to catch the redirect
    // The URL it listens for is http://127.0.0.1:LISTEN_PORT/cb
    auto replyHandler = new QOAuthHttpServerReplyHandler(LISTEN_PORT, this);
    m_oauth.setReplyHandler(replyHandler);

    // 5. Connect signals to handle the flow stages

    // This signal is emitted when the OAuth flow needs the user to authorize via browser
    connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    // This signal is emitted when tokens are successfully exchanged
    connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::granted, this, &GoogleAuthenticator::onGranted);

    // This signal is emitted on any error
    connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::error, this, &GoogleAuthenticator::onError);

    qDebug() << "Authenticator initialized. Starting authorization flow...";
}

void GoogleAuthenticator::startAuthentication()
{
    Q_EMIT started();

    // Check if we already have a token saved (e.g., from a previous session)
    if (loadToken()) {
        qDebug() << "Successfully loaded existing token. Ready to use API.";
        return;
    }

    // If no token exists, start the authorization process
    qDebug() << "Requesting new grant...";
    m_oauth.grant();
}

QString GoogleAuthenticator::token() const
{
    return m_oauth.token();
}

bool GoogleAuthenticator::hasToken() const
{
    return QFileInfo::exists("tokens.json");
}

void GoogleAuthenticator::onGranted()
{
    qDebug() << "--- OAuth Granted Successfully! ---";
    QString accessToken = m_oauth.token();
    QString refreshToken = m_oauth.refreshToken();

    qDebug() << "Access Token:" << accessToken.left(10) << "... (Hidden for security)";
    qDebug() << "Refresh Token:" << refreshToken.left(10) << "... (Hidden for security)";

    saveToken(); // Persist the tokens for future use

    // Get the token expiration in seconds (QAbstractOAuth2::expirationAt returns QDateTime)
    // Since we need the *duration* until expiration, we'll calculate it from the QAbstractOAuth2::expirationAt()
    qint64 expiresInSeconds = QDateTime::currentDateTime().secsTo(m_oauth.expirationAt());

    // --- Token Refresh Logic ---
    const int leadTimeSeconds = 300; // 5 minutes lead time

    if (expiresInSeconds > leadTimeSeconds) {
        int refreshDelayMs = (expiresInSeconds - leadTimeSeconds) * 1000;

        qDebug() << QString("Token expires in %1 seconds. Setting refresh timer for %2 seconds.")
                        .arg(expiresInSeconds)
                        .arg(refreshDelayMs / 1000);

        // Set the new interval and start the timer
        m_refreshTimer.setSingleShot(true); // Ensure it only runs once per token lifetime
        m_refreshTimer.start(refreshDelayMs);
    } else {
        qWarning() << "Token expires too soon or is already expired. Refreshing immediately.";
        // If the time is too short, refresh immediately
        m_oauth.refreshAccessToken();
    }

    Q_EMIT authenticated();
}

void GoogleAuthenticator::onError(const QString &error, const QString &errorDescription, const QUrl &uri)
{
    Q_UNUSED(uri);
    qCritical() << "--- OAuth Error Occurred! ---";
    qCritical() << "Error:" << error;
    qCritical() << "Description:" << errorDescription;
    QCoreApplication::exit(1);
}

void GoogleAuthenticator::saveToken()
{
    QJsonObject tokenData;
    tokenData["access_token"] = m_oauth.token();
    tokenData["refresh_token"] = m_oauth.refreshToken();
    //        tokenData["expiration_date"] = QDateTime::currentDateTime().addSecs(m_oauth.expiresIn()).toString(Qt::ISODate);

    QFile file("tokens.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(tokenData).toJson());
        qDebug() << "Tokens saved to tokens.json";
        file.close();
    } else {
        qWarning() << "Could not open tokens.json for writing!";
    }
}

bool GoogleAuthenticator::loadToken()
{
    QFile file("tokens.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject tokenData = doc.object();

    if (tokenData.contains("access_token") && tokenData.contains("refresh_token")) {
        m_oauth.setToken(tokenData["access_token"].toString());
        m_oauth.setRefreshToken(tokenData["refresh_token"].toString());

        // You would typically also check the expiration date and call m_oauth.refreshAccessToken()
        // if the token is near expiration or if an API call fails with a 401 error.
        m_oauth.refreshAccessToken();

        return true;
    }
    return false;
}
