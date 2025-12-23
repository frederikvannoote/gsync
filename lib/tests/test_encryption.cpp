#include <QtTest>
#include "keymanager.h"
#include "encryption.h"
#include <QBuffer>

class TestEncryption : public QObject
{
    Q_OBJECT

private slots:
    void testKeyDerivation();
    void testKeyDerivationArgon2();
    void testEncryptDecrypt();
    void testEncryptDecryptWrongKey();
    void testDecryptTruncated();
    void testEncryptDecryptLargeChunking();
    void testEmptyPlaintext();
    void testHeaderCorruption();
    void testHeaderVersionMismatch();
    void testSmallChunkSize();
};

void TestEncryption::testKeyDerivation()
{
    QByteArray salt = QByteArray::fromHex("00112233445566778899aabbccddeeff");
    QByteArray k1 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("correct horse battery staple"), salt, 32);
    QByteArray k2 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("correct horse battery staple"), salt, 32);
    QCOMPARE(k1, k2);
    QCOMPARE(k1.size(), 32);
}

void TestEncryption::testKeyDerivationArgon2()
{
    // Only meaningful when libsodium is available
#ifdef HAVE_LIBSODIUM
    QByteArray salt1 = QByteArray::fromHex("00112233445566778899aabbccddeeff");
    QByteArray salt2 = QByteArray::fromHex("ffeeddccbbaa99887766554433221100");
    QByteArray k1 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("hunter2"), salt1, 32);
    QByteArray k2 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("hunter2"), salt1, 32);
    QByteArray k3 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("hunter2"), salt2, 32);
    QCOMPARE(k1.size(), 32);
    QCOMPARE(k1, k2);
    QVERIFY(k1 != k3);
#else
    QSKIP("libsodium not available; skipping Argon2 derivation tests.");
#endif
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

void TestEncryption::testEncryptDecryptWrongKey()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping wrong-key test.");

    QByteArray plaintext = "Hello, world!";
    QByteArray key1 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("pw1"), QByteArray("salt"), 32);
    QByteArray key2 = KeyManager::deriveKeyFromPassphrase(QStringLiteral("pw2"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key1, params));

    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(!Encryption::decryptStream(cipherIn, restoredOut, key2, params));
}

void TestEncryption::testDecryptTruncated()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping truncated test.");

    QByteArray plaintext = QByteArray(1024, 'A');
    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    params.chunkSize = 512;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

    // Truncate the ciphertext (drop last 16 bytes)
    QByteArray truncated = ciphertext.left(ciphertext.size() - 16);
    QBuffer truncatedIn(&truncated);
    truncatedIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(!Encryption::decryptStream(truncatedIn, restoredOut, key, params));
}

void TestEncryption::testEncryptDecryptLargeChunking()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping large chunking test.");

    // Create a large plaintext spanning multiple chunks
    QByteArray plaintext;
    const int chunkSize = 1024;
    for (int i = 0; i < chunkSize * 3 + 123; ++i) plaintext.append(char('A' + (i % 26)));

    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    params.chunkSize = chunkSize;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(Encryption::decryptStream(cipherIn, restoredOut, key, params));
    QCOMPARE(restored, plaintext);
}

void TestEncryption::testEmptyPlaintext()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping empty plaintext test.");

    QByteArray plaintext;
    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));
    // Ciphertext should contain header only
    QVERIFY(ciphertext.size() >= 9);

    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(Encryption::decryptStream(cipherIn, restoredOut, key, params));
    QCOMPARE(restored.size(), 0);
}

void TestEncryption::testHeaderCorruption()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping header corruption test.");

    QByteArray plaintext = "data";
    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

    // Corrupt the magic header
    ciphertext[0] ^= 0xFF;

    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(!Encryption::decryptStream(cipherIn, restoredOut, key, params));
}

void TestEncryption::testHeaderVersionMismatch()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping header version mismatch test.");

    QByteArray plaintext = "x";
    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

    // bump version byte
    ciphertext[4] = 0x02;

    QBuffer cipherIn(&ciphertext);
    cipherIn.open(QIODevice::ReadOnly);
    QByteArray restored;
    QBuffer restoredOut(&restored);
    restoredOut.open(QIODevice::WriteOnly);

    QVERIFY(!Encryption::decryptStream(cipherIn, restoredOut, key, params));
}

void TestEncryption::testSmallChunkSize()
{
    if (!Encryption::isAvailable()) QSKIP("Encryption backend not available; skipping small chunk size test.");

    QByteArray plaintext;
    for (int i = 0; i < 1000; ++i) plaintext.append(char('a' + (i % 26)));

    QByteArray key = KeyManager::deriveKeyFromPassphrase(QStringLiteral("password"), QByteArray("salt"), 32);

    QBuffer inBuf(&plaintext);
    inBuf.open(QIODevice::ReadOnly);

    QByteArray ciphertext;
    QBuffer outBuf(&ciphertext);
    outBuf.open(QIODevice::WriteOnly);

    Encryption::Params params;
    params.chunkSize = 1;
    QVERIFY(Encryption::encryptStream(inBuf, outBuf, key, params));

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
