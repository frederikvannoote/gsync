#include "passworddialog.h"
#include "ui_passworddialog.h"

#include <QDialogButtonBox>

PasswordDialog::PasswordDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

QString PasswordDialog::passphrase() const
{
    return ui->passEdit->text();
}

bool PasswordDialog::remember() const
{
    return ui->rememberCheckBox->isChecked();
}

bool PasswordDialog::getPassphrase(QWidget *parent, QString &outPassphrase, bool &outRemember)
{
    PasswordDialog dlg(parent);
    if (dlg.exec() == QDialog::Accepted) {
        outPassphrase = dlg.passphrase();
        outRemember = dlg.remember();
        return true;
    }
    return false;
}
