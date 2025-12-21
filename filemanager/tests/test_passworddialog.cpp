#include <QtTest>
#include "passworddialog.h"

class TestPasswordDialog : public QObject
{
    Q_OBJECT

private slots:
    void testGetPassphrase();
};

void TestPasswordDialog::testGetPassphrase()
{
    // We will test the convenience API by creating the dialog and simulating input
    QString pass;
    bool remember = false;

    // Create dialog directly and set fields, then accept
    PasswordDialog dlg;
    dlg.show();
    QTest::keyClicks(dlg.findChild<QLineEdit*>("passEdit"), "hunter2");
    QTest::mouseClick(dlg.findChild<QDialogButtonBox*>("buttonBox")->button(QDialogButtonBox::Ok), Qt::LeftButton);

    // Now read fields
    pass = dlg.passphrase();
    remember = dlg.remember();

    QCOMPARE(pass, QString("hunter2"));
    QCOMPARE(remember, false);
}

QTEST_MAIN(TestPasswordDialog)
#include "test_passworddialog.moc"
