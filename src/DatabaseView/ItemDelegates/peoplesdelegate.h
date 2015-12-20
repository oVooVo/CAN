#ifndef PEOPLESDELEGATE_H
#define PEOPLESDELEGATE_H

#include "itemdelegate.h"

class PeoplesEdit : public QWidget
{
    Q_OBJECT
public:
    PeoplesEdit(QWidget* parent) :
        QWidget(parent)
    {

    }

};

class PeoplesDelegate : public ItemDelegate<PeoplesEdit>
{
public:
    PeoplesDelegate(QObject *parent = nullptr);

    void setEditorData(PeoplesEdit* editor, const QModelIndex& index) const;
    void setModelData(PeoplesEdit *editor, QAbstractItemModel *database, const QModelIndex &index) const;
};

#endif // PEOPLESDELEGATE_H