#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "googleauthenticator.h"
#include "googlefilelist.h"
#include "googledrive.h"
#include "googlesync.h"
#include "googlefilesyncmodel.h"
#include "statusfilterproxymodel.h"
#include "progressdelegate.h"
#include <QFileDialog>
#include <QToolBar>


MainWindow::MainWindow(GoogleAuthenticator &authenticator,
                       GoogleFileList &files,
                       GoogleDrive &drive,
                       GoogleSync &sync,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_authenticator(authenticator)
    , m_files(files)
    , m_drive(drive)
    , m_sync(sync)
    , m_pRunningSyncs(nullptr)
    , m_pToAnalyze(nullptr)
    , m_pSyncedFiles(nullptr)
{
    ui->setupUi(this);
    createToolBar();
    createStatusBar();
    menuBar()->hide();

    connect(&drive, &GoogleDrive::fileDiscoveryStarted, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->discovery);
    });
    connect(&sync, &GoogleSync::started, this, [this]() {
        menuBar()->show();

        GoogleFileSyncModel *model = new GoogleFileSyncModel(this);
        StatusFilterProxyModel *proxyModel = new StatusFilterProxyModel(this);
        proxyModel->setDynamicSortFilter(true);
        proxyModel->setSourceModel(model);

        connect(ui->actionShowFilesWithUnknownStatus, &QAction::triggered, this, [proxyModel](bool checked) {
            proxyModel->setFilter(StatusFilterProxyModel::FilterUnknown, checked);
        });
        connect(ui->actionShowInSyncFiles, &QAction::triggered, this, [proxyModel](bool checked) {
            proxyModel->setFilter(StatusFilterProxyModel::FilterSynced, checked);
        });
        connect(ui->actionShowOutOfSyncFiles, &QAction::triggered, this, [proxyModel](bool checked) {
            proxyModel->setFilter(StatusFilterProxyModel::FilterNeedsSyncing, checked);
        });
        connect(ui->actionShowSyncingFiles, &QAction::triggered, this, [proxyModel](bool checked) {
            proxyModel->setFilter(StatusFilterProxyModel::FilterSyncing, checked);
        });

        QVector<GoogleFileSync*> fs = m_sync.files();
        model->setFiles(fs);
        ui->tableView->setModel(proxyModel);
        ui->tableView->setItemDelegateForColumn(GoogleFileSyncModel::Progress, new ProgressDelegate(ui->tableView));
        ui->tableView->setSortingEnabled(true);

        ui->stackedWidget->setCurrentWidget(ui->sync);
    });
    connect(&sync, &GoogleSync::numberOfFilesChanged, this, [this](int files) {
        m_pSyncedFiles->setMaximum(files);
    });
    connect(&sync, &GoogleSync::runningSyncsChanged, this, [this](int runningSyncs) {
        m_pRunningSyncs->setText(tr("Running syncs: %1").arg(runningSyncs));
    });
    connect(&sync, &GoogleSync::unknownItemsChanged, this, [this](int unknown) {
        m_pToAnalyze->setText(tr("%1 files to analyze").arg(unknown));
    });
    connect(&sync, &GoogleSync::syncedItemsChanged, this, [this](int synced) {
        m_pSyncedFiles->setValue(synced);
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::indicateAuthenticationStart()
{
    ui->connectStatus->setVisible(false);
    ui->start->setEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->config);
    ui->connect->setVisible(!m_authenticator.hasToken());
    if (m_authenticator.hasToken())
        m_authenticator.startAuthentication();
}

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    statusBar->hide(); // Hidden at start
    connect(&m_sync, &GoogleSync::started, this, [statusBar]() {
        statusBar->show();
    });

    m_pRunningSyncs = new QLabel(this);
    statusBar->addPermanentWidget(m_pRunningSyncs);

    m_pToAnalyze = new QLabel(this);
    statusBar->addPermanentWidget(m_pToAnalyze);

    m_pSyncedFiles = new QProgressBar(this);
    statusBar->addPermanentWidget(m_pSyncedFiles, 1);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar(tr("Filters"));
    toolBar->hide(); // Hidden at start
    connect(&m_sync, &GoogleSync::started, this, [toolBar]() {
        toolBar->show();
    });

    toolBar->addAction(ui->actionShowFilesWithUnknownStatus);
    toolBar->addAction(ui->actionShowOutOfSyncFiles);
    toolBar->addAction(ui->actionShowSyncingFiles);
    toolBar->addAction(ui->actionShowInSyncFiles);
}
