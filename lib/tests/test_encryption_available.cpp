#include <QtTest>
#include "encryption.h"

class TestEncryptionAvailable : public QObject
{
    Q_OBJECT

private slots:
    void testAvailable();
};

void TestEncryptionAvailable::testAvailable()
{
    QVERIFY(Encryption::isAvailable());
}

QTEST_MAIN(TestEncryptionAvailable)
#include "test_encryption_available.moc"
