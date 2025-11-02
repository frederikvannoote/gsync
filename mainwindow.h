#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>

namespace Ui {
class MainWindow;
}
class GoogleAuthenticator;
class GoogleFileList;
class GoogleDrive;
class GoogleSync;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GoogleAuthenticator &authenticator,
                        GoogleFileList &files,
                        GoogleDrive &drive,
                        GoogleSync &sync,
                        QWidget *parent = nullptr);
    ~MainWindow();

public Q_SLOTS:
    void indicateAuthenticationStart();

private:
    void createStatusBar();
    void createToolBar();

    Ui::MainWindow *ui;
    GoogleAuthenticator &m_authenticator;
    GoogleFileList &m_files;
    GoogleDrive &m_drive;
    GoogleSync &m_sync;

    QLabel *m_pRunningSyncs;
    QLabel *m_pToAnalyze;
    QProgressBar *m_pSyncedFiles;
};
