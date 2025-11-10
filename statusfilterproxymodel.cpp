#include "statusfilterproxymodel.h"
#include "googlefilesyncmodel.h" // Required to access the model structure
#include "googlefilesync.h"


StatusFilterProxyModel::StatusFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_currentFilterState(FilterAll) // Default to showing all files
{
    // The filter accepts all rows by default until a state is set.
}

void StatusFilterProxyModel::setFilterStatus(FilterState state)
{
    if (m_currentFilterState != state)
    {
        m_currentFilterState = state;
        // The invalidate filter signal forces Qt to re-run filterAcceptsRow() for all rows.
        invalidateFilter();
    }
}

void StatusFilterProxyModel::setFilter(FilterStateFlag flag, bool checked)
{
    m_currentFilterState.setFlag(flag, checked);
    invalidateFilter();
}

bool StatusFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // If set to FilterAll, accept every row immediately.
    if (m_currentFilterState == FilterAll) {
        return true;
    }

    // Get the index for the Status column (assuming this is where your state is stored)
    // You MUST know the column number where the status data is located in your source model.
    QModelIndex statusIndex = sourceModel()->index(sourceRow, GoogleFileSyncModel::Status, sourceParent);

    // Retrieve the raw state value using the CustomRole if available,
    // or just the DisplayRole string if that's what your model exposes.
    // Assuming your GoogleFileSyncModel exposes the raw State enum value via Qt::UserRole + 1 (as is common).
    QVariant data = sourceModel()->data(statusIndex, GoogleFileSyncModel::CustomRoles::RawStatusRole);

    // Safety check: if data isn't valid, don't show the row.
    if (!data.isValid()) {
        return false;
    }

    // Convert the QVariant back to the GoogleFileSync::State enum value
    GoogleFileSync::State fileState = static_cast<GoogleFileSync::State>(data.toInt());
    FilterStateFlag fileFilterFlag = FilterStateFlag::FilterUnknown;
    switch (fileState)
    {
    case GoogleFileSync::State::ANALYZING:
        fileFilterFlag = FilterStateFlag::FilterAnalyzing;
        break;
    case GoogleFileSync::State::OUTOFSYNC:
        fileFilterFlag = FilterStateFlag::FilterNeedsSyncing;
        break;
    case GoogleFileSync::State::SYNCING:
        fileFilterFlag = FilterStateFlag::FilterSyncing;
        break;
    case GoogleFileSync::State::SYNCED:
        fileFilterFlag = FilterStateFlag::FilterSynced;
        break;
    case GoogleFileSync::State::UNKNOWN:
    default:
        fileFilterFlag = FilterStateFlag::FilterUnknown;
        break;
    }

    return m_currentFilterState.testFlag(fileFilterFlag);
}
