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

    GoogleFileSyncModel *model = new GoogleFileSyncModel(this);
    StatusFilterProxyModel *proxyModel = new StatusFilterProxyModel(this);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(model);

    connect(ui->actionShowFilesWithUnknownStatus, &QAction::triggered, this, [proxyModel](bool checked) {
        proxyModel->setFilter(StatusFilterProxyModel::FilterUnknown, checked);
    });
    connect(ui->actionShowBeingAnalyzed, &QAction::triggered, this, [proxyModel](bool checked) {
        proxyModel->setFilter(StatusFilterProxyModel::FilterAnalyzing, checked);
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = this->statusBar();

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

    toolBar->addAction(ui->actionShowFilesWithUnknownStatus);
    toolBar->addAction(ui->actionShowBeingAnalyzed);
    toolBar->addAction(ui->actionShowOutOfSyncFiles);
    toolBar->addAction(ui->actionShowSyncingFiles);
    toolBar->addAction(ui->actionShowInSyncFiles);
}
