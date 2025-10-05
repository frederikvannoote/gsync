#pragma once

#include <QStyledItemDelegate>

class ProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ProgressDelegate(QObject *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
