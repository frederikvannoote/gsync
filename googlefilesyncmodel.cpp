#include "googlefilesyncmodel.h"
#include "googlefilesync.h"


GoogleFileSyncModel::GoogleFileSyncModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void GoogleFileSyncModel::setFiles(const QVector<GoogleFileSync*> &files)
{
    // Important: Tells the view that the underlying data structure is about to change entirely.
    beginResetModel();
    m_files = files;
    for (auto fs: files)
    {
        connect(fs, &GoogleFileSync::stateChanged, this, &GoogleFileSyncModel::onFileSyncStatusUpdate);
        connect(fs, &GoogleFileSync::progressChanged, this, &GoogleFileSyncModel::onFileProgressChanged);
    }
    endResetModel();
}

int GoogleFileSyncModel::rowCount(const QModelIndex &parent) const
{
    // Models should not have children, so if parent is valid, row count is 0.
    if (parent.isValid())
        return 0;

    return m_files.count();
}

int GoogleFileSyncModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return Columns::ColumnCount;
}

QVariant GoogleFileSyncModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_files.size() || index.row() < 0)
        return QVariant();

    const GoogleFileSync *f = m_files.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Columns::Name:
            return f->file().name();
        case Columns::Path:
            return f->file().path();
        case Columns::Progress:
            if (f->state() == GoogleFileSync::SYNCING)
                return QString("%1%").arg(f->progress());
            else
                return QString();
        case Columns::Status:
            return GoogleFileSync::toString(f->state());
        }
    }

    if (role == CustomRoles::ProgressRole) { // Raw percentage for delegate painting
        return f->progress();
    }

    return QVariant();
}

QVariant GoogleFileSyncModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case Columns::Name:
            return QStringLiteral("File Name");
        case Columns::Path:
            return QStringLiteral("Path");
        case Columns::Progress:
            return QStringLiteral("Progress");
        case Columns::Status:
            return QStringLiteral("Status");
        default:
            break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

/**
 * @brief Implements native sorting for the model.
 */
void GoogleFileSyncModel::sort(int column, Qt::SortOrder order)
{
    // 1. Prepare the model for a data change notification
    beginResetModel();

    // 2. Use std::sort with a custom comparison lambda
    std::sort(m_files.begin(), m_files.end(),
              [&](const GoogleFileSync *a, const GoogleFileSync *b) {

                  if (!a && !b) return false;
                  if (!a) return (order == Qt::AscendingOrder);
                  if (!b) return (order != Qt::AscendingOrder);

                  QString strA, strB;
                  qint64 numA, numB;
                  bool isNumeric = false;

                  // Get the raw data for comparison based on column
                  switch (column) {
                  case Columns::Progress: // Sort by percentage
                      isNumeric = true;
                      numA = a->progress();
                      numB = b->progress();
                      break;
                  case Columns::Name:
                      isNumeric = false;
                      strA = a->file().name();
                      strB = b->file().name();
                      break;
                  case Columns::Path:
                      isNumeric = false;
                      strA = a->file().path();
                      strB = b->file().path();
                      break;
                  case Columns::Status:
                      isNumeric = true;
                      numA = a->state();
                      numB = b->state();
                      break;
                      // No default needed since all Columns are handled.
                  }

                  bool lessThan = false;
                  if (isNumeric) {
                      if (numA == numB) return false; // Explicitly handle equality
                      bool lessThan = numA < numB;
                      return (order == Qt::AscendingOrder) ? lessThan : !lessThan;
                  } else {
                      int comparisonResult = strA.localeAwareCompare(strB);

                      if (comparisonResult == 0) return false; // Explicitly handle equality

                      bool lessThan = comparisonResult < 0;
                      return (order == Qt::AscendingOrder) ? lessThan : !lessThan;
                  }

                  // 4. Apply the sort order
                  return (order == Qt::AscendingOrder) ? lessThan : !lessThan;
              });

    // 3. Notify the views that the data has been sorted
    endResetModel();
}

void GoogleFileSyncModel::onFileSyncStatusUpdate()
{
    GoogleFileSync* sender = qobject_cast<GoogleFileSync*>(this->sender());

    for (int row = 0; row < m_files.size(); ++row)
    {
        GoogleFileSync *f = m_files.at(row);
        if (f->file().id() == sender->file().id())
        {
            QModelIndex topLeft = index(row, Columns::Name);
            QModelIndex bottomRight = index(row, Columns::Status);

            emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, Qt::TextAlignmentRole, CustomRoles::RawSizeRole});

            return;
        }
    }
}

void GoogleFileSyncModel::onFileProgressChanged(int value)
{
    GoogleFileSync* sender = qobject_cast<GoogleFileSync*>(this->sender());

    for (int row = 0; row < m_files.size(); ++row)
    {
        GoogleFileSync *f = m_files.at(row);
        if (f->file().id() == sender->file().id())
        {
            QModelIndex topLeft = index(row, Columns::Progress);
            QModelIndex bottomRight = index(row, Columns::Progress);

            emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, CustomRoles::ProgressRole});

            return;
        }
    }
}
