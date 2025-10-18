#pragma once

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QTimer>


class GoogleAuthenticator : public QObject
{
    Q_OBJECT
public:
    explicit GoogleAuthenticator(const QString &client_id,
                                 const QString &client_secret,
                                 QObject *parent = nullptr);

    QString token() const;

    bool hasToken() const;

public Q_SLOTS:
    void startAuthentication();

Q_SIGNALS:
    void started();
    void authenticated();

private slots:
    void onGranted();

    void onError(const QString &error, const QString &errorDescription, const QUrl &uri);

    void saveToken();

    bool loadToken();

private:
    QOAuth2AuthorizationCodeFlow m_oauth;
    QTimer m_refreshTimer;
};

