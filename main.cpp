#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSettings>
#include "googleauthenticator.h"
#include "googledrive.h"
#include "googlefilelist.h"
#include "googlesync.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "GSync_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    const QString clientID = qEnvironmentVariable("GSYNC_CLIENT_ID", "YOUR_CLIENT_ID");
    const QString clientSecret = qEnvironmentVariable("GSYNC_CLIENT_SECRET", "YOUR_CLIENT_SECRET");

    // IMPORTANT: Check your configuration before running!
    if (clientID.contains("YOUR") || clientID.contains("YOUR")) {
        qCritical() << "ERROR: Please replace CLIENT_ID and CLIENT_SECRET in main.cpp with your actual credentials.";
        return 1;
    }

    GoogleAuthenticator auth(clientID, clientSecret);
    GoogleFileList files;
    GoogleDrive drive(files, auth);
    GoogleSync sync(drive);

    // Get the user authenticated.
    if (auth.hasToken() && auth.loadToken())
    {
        qInfo() << "Authenticate with existing token.";
    }
    else
    {
        QMessageBox::information(nullptr,
                                 QObject::tr("Welcome to GSync"),
                                 QObject::tr("To configure GSync, you need to start by authenticating to your Google Drive. Select the account which you would like to sync."));

        auth.grant();
    }

    // Ask where it needs to synchronize to.
    {
        QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "GSync");
        const QString destination = QFileDialog::getExistingDirectory(nullptr,
                                                                      QObject::tr("Choose directory where to sync to"),
                                                                      settings.value("DestinationDirectory", QString()).toString());
        if (destination.isEmpty())
            return 1;

        settings.setValue("DestinationDirectory", destination); // Save this for a next run.
        sync.setBaseDir(destination);

        QProgressDialog dialog(QObject::tr("Found %1 files.").arg(0),
                               QObject::tr("Cancel"), 0, 0);
        dialog.setWindowTitle(QObject::tr("Discovering files in Google Drive"));
        QObject::connect(&files, &GoogleFileList::countChanged, &dialog, [&dialog](int count) {
            dialog.setLabelText(QObject::tr("Found %1 files.").arg(count));
        });
        QObject::connect(&drive, &GoogleDrive::fileDiscoveryStopped, &dialog, [&dialog]() {
            dialog.accept();
        });

        drive.startFileDiscovery(); // Start reading file list of the remote Google Drive

        if (dialog.exec() == QProgressDialog::DialogCode::Rejected)
        {
            drive.stopFileDiscovery();
            return 2;
        }
    }

    // Prepare file synchronization
    for (GoogleFile file: files.files())
    {
        if (!file.type().startsWith("application/vnd.google-apps.")) // Google objects cannot be downloaded
        {
            QString path;
            GoogleFile f = file;
            while (!f.parents().isEmpty())
            {
                f = files.file(f.parents().first());
                if (f.isValid())
                {
                    path.prepend(f.name() + "/");
                }
            }

            //qInfo() << "Path of" << file.id() << file.name() << ":" << path;
            file.setPath(path);

            sync.add(file);
        }
    }

    // Start file synchronization
    MainWindow w(auth, files, drive, sync);
    sync.start();
    w.show();

    return a.exec();
}
