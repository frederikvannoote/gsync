#include <QtTest>
#include "keymanager.h"
#include "encryption.h"
#include <QBuffer>

class TestEncryption : public QObject
{
    Q_OBJECT

private slots:
    void testKeyDerivation();
    void testEncryptDecrypt();
};

void TestEncryption::testKeyDerivation()
{
    QByteArray salt = QByteArray::fromHex("00112233445566778899aabbccddeeff");
    QByteArray k1 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("correct horse battery staple"), salt, 32);
    QByteArray k2 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("correct horse battery staple"), salt, 32);
    QCOMPARE(k1, k2);
    QCOMPARE(k1.size(), 32);
}

void TestEncryption::testEncryptDecrypt()
{
    if (!Encryption::isAvailable()) {
        QSKIP("Encryption backend not available; skipping encrypt/decrypt roundtrip test.");
    }

    QByteArray plaintext = "The quick brown fox jumps over the lazy dog\n";
    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

    // Prepare to decrypt
    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(Encryption::decryptStream(cipherIn, restoredOut, key, params));
    QCOMPARE(restored, plaintext);
}

QTEST_MAIN(TestEncryption)
#include "test_encryption.moc"
