#pragma once

#include <QObject>
#include <QMap>
#include "googlefile.h"


class GoogleFileList : public QObject
{
    Q_OBJECT
public:
    explicit GoogleFileList(QObject *parent = nullptr);

    void add(GoogleFile &file);
    void remove(GoogleFile &file);

    GoogleFile file(const QString &id) const;

    QVector<GoogleFile> files() const;
    int count() const;

Q_SIGNALS:
    void added(GoogleFile &file);
    void removed(GoogleFile &file);

    void countChanged(int count);

private:
    QMap<QString, GoogleFile> m_files;
};
