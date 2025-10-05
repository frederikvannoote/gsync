#include "syncwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "googleauthenticator.h"
#include "googledrive.h"
#include "googlefilelist.h"
#include "googlesync.h"

// You must set up a "Desktop App" OAuth client.
const QString CLIENT_ID = "API_CLIENT_ID";
const QString CLIENT_SECRET = "API_KEY_PLACEHOLDER";

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

    // IMPORTANT: Check your configuration before running!
    if (CLIENT_ID.contains("YOUR") || CLIENT_SECRET.contains("YOUR")) {
        qCritical() << "ERROR: Please replace CLIENT_ID and CLIENT_SECRET in main.cpp with your actual credentials.";
        return 1;
    }

    GoogleAuthenticator auth(CLIENT_ID, CLIENT_SECRET);
    GoogleFileList files;
    GoogleDrive drive(files, auth);
    GoogleSync sync(drive);

    QObject::connect(&drive, &GoogleDrive::fileDiscoveryStopped, [&files, &sync](){
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

                qInfo() << "Path of" << file.id() << file.name() << ":" << path;
                file.setPath(path);

                sync.add(file);
            }
        }

        sync.start();
    });

    SyncWindow w(auth, files, drive, sync);
    w.indicateAuthenticationStart();
    w.show();

    return a.exec();
}
