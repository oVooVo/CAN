#include "songtableview.h"
#include "QHeaderView"
#include <QAction>
#include "global.h"
#include "project.h"
#include "renamableheaderview.h"
#include "util.h"
#include <QFocusEvent>
#include "application.h"


SongTableView::SongTableView(QWidget *parent) :
    DatabaseView(parent),
    m_delegate( new SongAttributeDelegate(this) )
{
    delete horizontalHeader();
    setHorizontalHeader(new RenamableHeaderView( Qt::Horizontal, this ));
    horizontalHeader()->show();

    setItemDelegate( m_delegate );

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setAlternatingRowColors(true);

    horizontalHeader()->setSectionsMovable(true);
    horizontalHeader()->setDragEnabled(true);
    horizontalHeader()->setDragDropMode(QAbstractItemView::InternalMove);
    horizontalHeader()->setSortIndicatorShown( true );
    horizontalHeader()->setSectionsClickable( true );
    connect(horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(fakeFocusOutEvent()));

    verticalHeader()->setSectionsMovable(true);
    verticalHeader()->setDragEnabled(true);
    verticalHeader()->setDragDropMode(QAbstractItemView::InternalMove);
    connect(verticalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(fakeFocusOutEvent()));

    setSortingEnabled( true );

    setDragDropMode( QAbstractItemView::DragDrop );
    setDragEnabled( true );
    setDropIndicatorShown( true );

    setContextMenuPolicy( Qt::NoContextMenu );

}


void SongTableView::setModel(SongDatabaseSortProxy *model)
{
    QTableView::setModel(model);
}

void SongTableView::fakeFocusOutEvent()
{
    // oddly, the columns in the table are not resized when the header is resized.
    // though, the colmns are resized when table loses focus.
    // since table can lose focus safely on resize, fire appropriate event to reach the
    // desired behaviour on the wrong track.
    QFocusEvent myFocusOutEvent(QEvent::FocusOut);
    QApplication::sendEvent( this, &myFocusOutEvent );
}

SongDatabase* SongTableView::model() const
{
    return proxyModel() ? proxyModel()->sourceModel() : NULL;
}

SongDatabaseSortProxy* SongTableView::proxyModel() const
{
    SongDatabaseSortProxy* pm = qobject_assert_cast<SongDatabaseSortProxy*>( QTableView::model() );
    assert( pm == QTableView::model() );
    return pm;
}

void SongTableView::setReadOnly()
{
    qobject_assert_cast<RenamableHeaderView*>( horizontalHeader() )->setReadOnly();
    setEditTriggers( QAbstractItemView::NoEditTriggers );
}

Qt::DropAction dropAction( QDropEvent* event )
{
    switch (event->keyboardModifiers())
    {
    case Qt::ControlModifier:
        return Qt::CopyAction;
    default:
        return Qt::MoveAction;
    }
}

void SongTableView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(SongDatabase::SONG_POINTERS_MIME_DATA_FORMAT) )
    {
        event->setDropAction( dropAction( event ) );
        event->accept();
    }
}

void SongTableView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(SongDatabase::SONG_POINTERS_MIME_DATA_FORMAT) )
    {
        event->setDropAction( dropAction( event ) );
        event->accept();
    }
}

#include "Commands/SongDatabaseCommands/songdatabaseremovesongcommand.h"
void SongTableView::dropEvent(QDropEvent *event)
{
    int row;
    QModelIndex index = indexAt( event->pos() );
    if (index.isValid())
    {
        row = index.row();
    }
    else
    {
        row = model()->rowCount();
    }

    model()->pasteSongs( event->mimeData(), row, dropAction( event ) );
}



