#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class SyncWindow;
}
QT_END_NAMESPACE
class GoogleAuthenticator;
class GoogleFileList;
class GoogleDrive;
class GoogleSync;


class SyncWindow : public QWidget
{
    Q_OBJECT

public:
    SyncWindow(GoogleAuthenticator &authenticator,
               GoogleFileList &files,
               GoogleDrive &drive,
               GoogleSync &sync,
               QWidget *parent = nullptr);
    ~SyncWindow();

public Q_SLOTS:
    void indicateAuthenticationStart();

private:
    Ui::SyncWindow *ui;
    GoogleAuthenticator &m_authenticator;
    GoogleFileList &m_files;
    GoogleDrive &m_drive;
    GoogleSync &m_sync;
};
