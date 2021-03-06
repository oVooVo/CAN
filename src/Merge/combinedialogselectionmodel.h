#ifndef COMBINEDIALOGSELECTIONMODEL_H
#define COMBINEDIALOGSELECTIONMODEL_H

#include <QItemSelectionModel>

class CombineDialogSelectionModel : public QItemSelectionModel
{
    // note: overriding of select is not very proper,
    // so CombineDialogSelectionModel::itemSelectionChanged may not work as expected.
    Q_OBJECT
public:
    explicit CombineDialogSelectionModel(QAbstractItemModel *parent = 0);

    void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command);
    void select(const QModelIndex &index, SelectionFlags command);
};

#endif // COMBINEDIALOGSELECTIONMODEL_H
