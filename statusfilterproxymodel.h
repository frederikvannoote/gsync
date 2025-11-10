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
        FilterAnalyzing = 0x1,
        FilterSyncing = 0x2,
        FilterSynced = 0x4,
        FilterNeedsSyncing = 0x8,
        FilterUnknown = 0x10,

        // Convenience flag: FilterAll is the combination of all possible sync states
        FilterAll = FilterSyncing | FilterSynced | FilterNeedsSyncing | FilterUnknown | FilterAnalyzing
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
