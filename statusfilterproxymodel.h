#pragma once

#include <QSortFilterProxyModel>


class StatusFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit StatusFilterProxyModel(QObject *parent = nullptr);

    // 1. Define the unique flag values (powers of 2)
    enum FilterStateFlag {
        FilterNone = 0x0,
        FilterSyncing = 0x1,
        FilterSynced = 0x2,
        FilterNeedsSyncing = 0x4, // Assuming this maps to GoogleFileSync::UNKNOWN
        FilterUnknown = 0x8,

        // Convenience flag: FilterAll is the combination of all possible sync states
        FilterAll = FilterSyncing | FilterSynced | FilterNeedsSyncing | FilterUnknown
    };
    Q_DECLARE_FLAGS(FilterState, FilterStateFlag) // Declare the QFlags type
    Q_FLAG(FilterState) // Register the new QFlags type with the meta-object system

    void setFilterStatus(FilterState state);
    void setFilter(FilterStateFlag flag, bool checked);

protected:
    // This is the core method for filtering. Returns true if the row should be shown.
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    FilterState m_currentFilterState;
};
