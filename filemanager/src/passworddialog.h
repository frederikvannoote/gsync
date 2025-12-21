#pragma once

#include <QDialog>

namespace Ui { class PasswordDialog; }

class PasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    ~PasswordDialog();

    QString passphrase() const;
    bool remember() const;

    // Convenience static helper
    static bool getPassphrase(QWidget *parent, QString &outPassphrase, bool &outRemember);

private:
    Ui::PasswordDialog *ui;
};
