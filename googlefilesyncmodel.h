#pragma once

#include <QAbstractTableModel>
#include <QVector>
class GoogleFileSync;


/**
 * @brief Qt Model implementation for displaying GoogleFile data in a QTableView.
 * * This class inherits QAbstractTableModel and adapts the QVector<GoogleFile>
 * data structure for the Qt Model/View framework.
 */
class GoogleFileSyncModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // Define custom roles for data access (e.g., getting the raw ID for logic)
    enum CustomRoles {
        FileIdRole = Qt::UserRole + 1,
        RawSizeRole, // Raw qint64 size for logic/sorting
        ProgressRole // Raw integer percentage (0-100) for the progress bar delegate
    };

    // Define the columns available in the table view using an enum for clarity
    enum Columns {
        Name = 0,
        Path,
        Progress,
        Status,
        ColumnCount
    };

    explicit GoogleFileSyncModel(QObject *parent = nullptr);

    /**
     * @brief Updates the model's underlying data.
     * * NOTE: beginResetModel() and endResetModel() are critical for notifying the view.
     * @param files The vector of GoogleFile objects to display.
     */
    void setFiles(const QVector<GoogleFileSync*> &files);

    void updateFile(const QString &id, qint64 newSize, const QString &newMd5Sum);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private Q_SLOTS:
    void onFileSyncStatusUpdate();
    void onFileProgressChanged(int value);

private:
    QVector<GoogleFileSync*> m_files;
};
