#ifndef RENAMABLEHEADERVIEW_H
#define RENAMABLEHEADERVIEW_H

#include <QHeaderView>
#include <QTableView>
#include "songtableview.h"

class RenamableHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit RenamableHeaderView( Qt::Orientation orientation, SongTableView *parent = 0);
    SongTableView* parent() const { return qobject_cast<SongTableView*>( QHeaderView::parent() ); }

public slots:
    void editHeader(int section , bool endMacroOnFinish = false);

protected:
   void mouseDoubleClickEvent(QMouseEvent *event);


private slots:
    void showContextMenu(QPoint pos);

private:
    void setUpContextMenu(QMenu* menu);
    int sectionUnderCursor() const;

};

#endif // RENAMABLEHEADERVIEW_H