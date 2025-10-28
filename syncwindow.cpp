#include "syncwindow.h"
#include "./ui_syncwindow.h"
#include "googleauthenticator.h"
#include "googlefilelist.h"
#include "googledrive.h"
#include "googlesync.h"
#include "googlefilesyncmodel.h"
#include "progressdelegate.h"
#include <QFileDialog>


SyncWindow::SyncWindow(GoogleAuthenticator &authenticator,
                       GoogleFileList &files,
                       GoogleDrive &drive,
                       GoogleSync &sync,
                       QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SyncWindow)
    , m_authenticator(authenticator)
    , m_files(files)
    , m_drive(drive)
    , m_sync(sync)
{
    ui->setupUi(this);

    connect(&drive, &GoogleDrive::fileDiscoveryStarted, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->discovery);
    });
    connect(&sync, &GoogleSync::started, this, [this]() {
        GoogleFileSyncModel *model = new GoogleFileSyncModel(this);
        QVector<GoogleFileSync*> fs = m_sync.files();
        model->setFiles(fs);
        ui->tableView->setModel(model);
        ui->tableView->setItemDelegateForColumn(GoogleFileSyncModel::Progress, new ProgressDelegate(ui->tableView));
        ui->tableView->setSortingEnabled(true);

        ui->stackedWidget->setCurrentWidget(ui->sync);
    });
    connect(&sync, &GoogleSync::numberOfFilesChanged, this, [this](int files) {
        ui->syncedFiles->setMaximum(files);
    });
    connect(&sync, &GoogleSync::runningSyncsChanged, this, [this](int runningSyncs) {
        ui->runningSyncs->setText(tr("Running syncs: %1").arg(runningSyncs));
    });
    connect(&sync, &GoogleSync::unknownItemsChanged, this, [this](int unknown) {
        ui->toAnalyze->setText(tr("%1 files to analyze").arg(unknown));
    });
    connect(&sync, &GoogleSync::syncedItemsChanged, this, [this](int synced) {
        ui->syncedFiles->setValue(synced);
    });
    connect(&files, &GoogleFileList::countChanged, ui->numberOfFiles, [this](int count) {
        ui->numberOfFiles->setText(tr("Found %1 files").arg(count));
    });
    connect(&authenticator, &GoogleAuthenticator::started, ui->connect, [this](){
        ui->connect->setEnabled(false);
    });
    connect(&authenticator, &GoogleAuthenticator::authenticated, ui->connectStatus, &QLabel::show);
    connect(ui->cancel, &QPushButton::clicked, &drive, &GoogleDrive::stopFileDiscovery);
    connect(ui->connect, &QPushButton::clicked, &authenticator, &GoogleAuthenticator::startAuthentication);
    connect(ui->start, &QPushButton::clicked, &drive, &GoogleDrive::startFileDiscovery);
    connect(ui->dirSelect, &QToolButton::clicked, ui->dirSelect, [this](){
        const QString d = QFileDialog::getExistingDirectory(this, tr("Select target directory"));
        ui->dirEdit->setText(d);
        m_sync.setBaseDir(d);
        ui->start->setEnabled(!d.isEmpty());
    });
}

SyncWindow::~SyncWindow()
{
    delete ui;
}

void SyncWindow::indicateAuthenticationStart()
{
    ui->connectStatus->setVisible(false);
    ui->start->setEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->config);
    ui->connect->setVisible(!m_authenticator.hasToken());
    if (m_authenticator.hasToken())
        m_authenticator.startAuthentication();
}
