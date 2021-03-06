#include "setlistwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>

#include "PDFCreator/pdfcreator.h"
#include "Commands/DatabaseCommands/databasenewitemcommand.h"
#include "ui_setlistwidget.h"
#include "Database/SongDatabase/songdatabase.h"
#include "Project/project.h"
#include "util.h"
#include "Database/EventDatabase/setlistitem.h"
#include "Database/EventDatabase/setlist.h"
#include "setlistitemselector.h"

SetlistWidget::SetlistWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetlistWidget),
    m_selector( new SetlistItemSelector( this ))
{
    ui->setupUi(this);

    connect(ui->setlistView, SIGNAL(clicked()), this, SLOT(updateButtonsEnabled()) );
    setSetlist( nullptr );

    m_attentionPixmap = QPixmap::fromImage(QImage(":/icons/icons/lightbulb20.png")).scaledToWidth( 32, Qt::SmoothTransformation );
    ui->infoLabelIcon->setFixedSize( 32, 32 );
}

SetlistWidget::~SetlistWidget()
{
    delete ui;
}

void SetlistWidget::updateButtonsEnabled()
{
    bool buttonsEnabled = !!setlistView()->model();

    ui->buttonAdd->setEnabled( buttonsEnabled );
    ui->buttonShowSongs->setEnabled( buttonsEnabled );
    ui->buttonExportPDF->setEnabled( buttonsEnabled );
}

void SetlistWidget::setSetlist(Database<SetlistItem> *setlist)
{
    if (setlistView()->model())
    {
        reConnect( setlistView()->model(), setlist, SIGNAL(rowsInserted(QModelIndex,int,int)),      this, SLOT(updateInfoLabel()) );
        reConnect( setlistView()->model(), setlist, SIGNAL(rowsRemoved(QModelIndex,int,int)),       this, SLOT(updateInfoLabel()) );
        reConnect( setlistView()->model(), setlist, SIGNAL(dataChanged(QModelIndex,QModelIndex)),   this, SLOT(updateInfoLabel()) );
    }

    ui->setlistView->setModel( setlist );
    updateInfoLabel();
    updateButtonsEnabled();
}


bool ascending(const QModelIndex& a, const QModelIndex& b)
{
    return a < b;
}

bool descending(const QModelIndex& a, const QModelIndex& b)
{
    return !ascending(a, b);
}

void SetlistWidget::on_buttonAdd_clicked()
{
    if (setlist())
    {
        pushCommand( new DatabaseNewItemCommand<SetlistItem>( setlist(), new SetlistItem(setlist()) ) );
    }
    else
    {
        qWarning() << "Cannot add Item to no set list.";
    }
}

SetlistView* SetlistWidget::setlistView() const
{
    return ui->setlistView;
}

void SetlistWidget::on_buttonShowSongs_clicked()
{
    m_selector->show();
}

void SetlistWidget::hideEvent(QHideEvent *e)
{
    m_selectorIsVisible = m_selector->isVisible();

    m_selector->hide();
    QWidget::hideEvent(e);
}

void SetlistWidget::showEvent(QShowEvent *e)
{
    if (m_selectorIsVisible)
    {
        m_selector->show();
    }
    updateInfoLabel();
    QWidget::showEvent(e);
}

Setlist* SetlistWidget::setlist() const
{
    return static_cast<Setlist*>(setlistView()->sourceModel());
}


void SetlistWidget::on_buttonExportPDF_clicked()
{
    Setlist* currentSetlist = setlist();
    if (currentSetlist)
    {
        PDFCreator::exportSetlist( currentSetlist, this );
    }
}

void SetlistWidget::updateInfoLabel()
{
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int songsWithValidDuration = 0;
    int allSongs = 0;
    Database<SetlistItem>* sl = setlist();

    bool containsInvalid = false;
    if (sl)
    {
        for (const SetlistItem* i : sl->items())
        {
            if (i->attribute("type").value<SetlistItem::Type>() == SetlistItem::SongType)
            {
                QTime duration = i->attribute("song").value<const Song*>()->attribute("duration").toTime();
                if (duration.isValid())
                {
                    seconds += duration.second();
                    minutes += duration.minute();
                    hours   += duration.hour();
                    songsWithValidDuration++;
                }
                else
                {
                    containsInvalid = true;
                }
                allSongs++;
            }
        }


        while (seconds >= 60)
        {
            seconds -= 60;
            minutes++;
        }

        while (minutes >= 60)
        {
            minutes -= 60;
            hours++;
        }

        QString h = QString("%1").arg(hours);
        QString m = QString("%1").arg(minutes);
        if (m.length() < 2) m = "0" + m;
        QString s = QString("%1").arg(seconds);
        if (s.length() < 2) s = "0" + s;

        QString toolTip = tr("Attention: %1 songs have no information about their duration.").arg( allSongs - songsWithValidDuration );
        if (containsInvalid)
        {
            ui->infoLabelIcon->setPixmap( m_attentionPixmap );
            ui->infoLabel->setToolTip( toolTip );
            ui->infoLabelIcon->setToolTip( toolTip );
        }
        else
        {
            ui->infoLabelIcon->setPixmap( QPixmap() );
            ui->infoLabelIcon->setToolTip( "" );
            ui->infoLabelIcon->setToolTip( "" );
        }
        ui->infoLabel->setText( QString(tr("%1 songs, duration: %2:%3:%4")).arg( allSongs ).arg( h, m, s ) );
    }
    else
    {
        ui->infoLabel->setText( "" );
    }
}
