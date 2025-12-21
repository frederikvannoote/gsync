#include <QtTest>
#include "syncfile.h"
#include <QFile>
#include <QJsonObject>

class TestSyncFile : public QObject
{
    Q_OBJECT

private slots:
    void testFromFileAndRestore();
};

void TestSyncFile::testFromFileAndRestore()
{
    // Create a temporary source file
    QString tempSource = QDir::temp().filePath("test_source.txt");
    QFile s(tempSource);
    QVERIFY(s.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray content("Hello GSync\n");
    s.write(content);
    s.close();

    // Create a SyncFile and store it into .gsync
    SyncFile file("id123", "test_source.txt", QCryptographicHash::hash(content, QCryptographicHash::Md5).toHex(), QDateTime::currentDateTime(), StorageFormat::RAW);
    QVERIFY(file.store(tempSource));

    QString gsyncPath = tempSource + ".gsync";
    QVERIFY(QFile::exists(gsyncPath));

    // Parse the .gsync
    SyncFile parsed = SyncFile::fromFile(gsyncPath);
    QCOMPARE(parsed.fileId(), QString("id123"));
    QCOMPARE(parsed.fileName(), QString("test_source.txt"));

    // Restore to another file
    QString restored = QDir::temp().filePath("test_restored.txt");
    QFile::remove(restored);
    QVERIFY(parsed.restore(restored));

    QFile r(restored);
    QVERIFY(r.open(QIODevice::ReadOnly));
    QByteArray restoredContent = r.readAll();
    r.close();
    QCOMPARE(restoredContent, content);

    // Cleanup
    QFile::remove(tempSource);
    QFile::remove(tempSource + ".gsync");
    QFile::remove(restored);
}

QTEST_MAIN(TestSyncFile)
#include "test_syncfile.moc"
