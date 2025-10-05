#include "progressdelegate.h"
#include "googlefilesyncmodel.h"
#include <QApplication>


ProgressDelegate::ProgressDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{}

void ProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 1. Check if the cell data is valid
    if (!index.isValid()) {
        return;
    }

    // 2. Read the raw progress percentage from the model using the custom role
    int progress = index.data(GoogleFileSyncModel::ProgressRole).toInt();

    // 3. If progress is not 100 (still downloading), draw the progress bar
    if (progress > 0 && progress < 100) {

        // Create a style option specific to a progress bar
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect.adjusted(2, 2, -2, -2); // Add padding
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QStringLiteral("%1%").arg(progress);
        progressBarOption.textVisible = true;

        // Use the application's style to draw the control
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

    } else if (progress == 100) {

        // If completed, just draw the "Done (Verified)" text using the base delegate's painter
        // The model returns the finished status text via Qt::DisplayRole (see model implementation)
        QStyledItemDelegate::paint(painter, option, index);

    } else {
        // For 0 (Pending) or errors, just use the standard delegate painting
        QStyledItemDelegate::paint(painter, option, index);
    }
}
