#include "mainwindow.h"

#include <QTimer>
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QProcess>
#include <QClipboard>
#include <QProgressDialog>
#include <QScrollArea>
#include <QLabel>
#include <QThread>
#include <QMimeData>
#include <QTemporaryFile>

#include "ui_mainwindow.h"
#include "util.h"
#include "Dialogs/stringdialog.h"
#include "Dialogs/programdialog.h"
#include "Commands/DatabaseCommands/databasenewitemcommand.h"
#include "Commands/DatabaseCommands/databaseeditcommand.h"
#include "Commands/DatabaseCommands/databaseremoveitemcommand.h"
#include "Commands/SongCommands/songremoveattachmentcommand.h"
#include "Commands/SongCommands/songnewattachmentcommand.h"
#include "Commands/AttachmentCommands/attachmentrenamecommand.h"
#include "Commands/SongCommands/songeditprogramcommand.h"
#include "AttachmentView/attachmentchooser.h"
#include "Attachments/attachment.h"
#include "Database/EventDatabase/setlistitem.h"
#include "Database/SongDatabase/songdatabase.h"
#include "Database/EventDatabase/eventdatabase.h"
#include "DatabaseView/EventDatabaseView/eventtableview.h"
#include "DatabaseView/SongDatabaseView/songtableview.h"
#include "creatable.h"
#include "DatabaseView/SongDatabaseView/songtableview.h"
#include "PDFCreator/orphantsetlist.h"
#include "PDFCreator/pdfcreator.h"
#include "DatabaseView/EventDatabaseView/setlistview.h"
#include "FileIndex/fileindex.h"
#include "Dialogs/copyindexedfilesdialog.h"
#include "AttachmentView/IndexedFileAttachmentView/indexedfileattachmentview.h"
#include "Attachments/indexedfileattachment.h"
#include "preferencedialog.h"
#include "Merge/mergedialog.h"
#include "application.h"
#include "Database/databasemimedata.h"
#include "Project/githandler.h"
#include "Dialogs/gitdialog.h"

QString styleSheetContent()
{
    QFile file(":/style/styles/stylesheet.qss");
    bool openFailed = !file.open(QIODevice::ReadOnly);

    Q_UNUSED( openFailed );
    Q_ASSERT( !openFailed );
    return file.readAll();
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    setWindowIcon( QIcon(":/icons/icons/01-elephant-icon.png") );
    app().setMainWindow( this );
    app().setProject( &m_project );
    createAttachmentActions();

    ui->setupUi(this);

    QActionGroup* viewGroup = new QActionGroup(this);
    viewGroup->addAction(ui->actionSongs);
    viewGroup->addAction(ui->actionSong_Details);
    viewGroup->addAction(ui->actionEvents);

    createLanguageMenu();

    QSettings settings;
    restoreGeometry( settings.value("Geometry").toByteArray() );

    //     load stylesheet
    app().setStyleSheet( styleSheetContent() );

    //////////////////////////////////////////
    /// restore Configurable
    //////////////////////////////////////////
    /// is done implicitly when Configurable is loaded


    menuBar()->setNativeMenuBar(false);
    setupAttachmentMenu();

    //////////////////////////////////////////
    /// Undo/Redo
    //////////////////////////////////////////
    connect( &m_project, SIGNAL(canUndoChanged(bool)), ui->actionUndo, SLOT(setEnabled(bool)));
    connect( &m_project, SIGNAL(canRedoChanged(bool)), ui->actionRedo, SLOT(setEnabled(bool)));

    auto action = [this]()
    {
        const QUndoCommand* nextUndoCommand = m_project.command( m_project.QUndoStack::index() - 1);
        const QUndoCommand* nextRedoCommand = m_project.command( m_project.QUndoStack::index() );

        QString undoText = tr("&Undo %1").arg( nextUndoCommand ? nextUndoCommand->actionText() : "" );
        QString redoText = tr("&Redo %1").arg( nextRedoCommand ? nextRedoCommand->actionText() : "" );

        undoAction()->setText( undoText );
        redoAction()->setText( redoText );
    };

    connect( &m_project, &Project::commandPushed, action );
    connect( &m_project, &Project::undoStackCleared, [this]()
    {
        undoAction()->setText( tr("&Undo") );
        redoAction()->setText( tr("&Redo") );
    });
    undoAction()->setText( tr("&Undo") );
    redoAction()->setText( tr("&Redo") );
    ui->actionUndo->setEnabled( m_project.canUndo() );
    ui->actionRedo->setEnabled( m_project.canRedo() );

    // very important to set associated widget. Else, shortcuts would be ambigous.
    initAction( actionNew_Song,         ui->songDatabaseWidget->databaseView(),    tr("&New Song"),       tr("Add a new song."),        "Ctrl+N",   ui->menuSongs,  "" )
    initAction( actionDelete_Song,      ui->songDatabaseWidget->databaseView(),    tr("&Remove Song"),    tr("Remove selected song."),  "Del",      ui->menuSongs,  ":/icons/icons/rubbish7.png" )
    initAction( actionCopy_Song,        ui->songDatabaseWidget->databaseView(),    tr("&Copy Song"),      tr("Copy selected song."),    "Ctrl+C",   ui->menuSongs,  "" )
    initAction( actionPaste_Song,       ui->songDatabaseWidget->databaseView(),    tr("&Paste Song"),     tr("Paste song."),            "Ctrl+V",   ui->menuSongs,  "" )

#ifdef HAVE_PROGRAM
    initAction( actionEdit_Program,     ui->songDatabaseWidget->databaseView(),    tr("&Edit Program"),   tr("Edit program."),          "Ctrl+P",   ui->menuSongs,  "" )
#endif

    initAction( actionNew_Event,        ui->eventDatabaseWidget->databaseView(),  tr("&New Event"),      tr("Add a new event."),        "Ctrl+N",   ui->menuEvents, "" )
    initAction( actionDelete_Event,     ui->eventDatabaseWidget->databaseView(),  tr("&Remove Event"),   tr("Remove selected event."),  "Del",      ui->menuEvents, ":/icons/icons/rubbish7.png" )
    initAction( actionCopy_Event,       ui->eventDatabaseWidget->databaseView(),  tr("&Copy Event"),     tr("Copy selected event."),    "Ctrl+C",   ui->menuEvents, "" )
    initAction( actionPaste_Event,      ui->eventDatabaseWidget->databaseView(),  tr("&Paste Event"),    tr("Paste event."),            "Ctrl+V",   ui->menuEvents, "" )

    initAction( actionNew_SetlistItem,    ui->eventDatabaseWidget->setlistView(),   tr("&New Item"),       tr("Insert new item"),        "Ctrl+N",   nullptr, "" )
    initAction( actionDelete_SetlistItem, ui->eventDatabaseWidget->setlistView(),   tr("&Remove Item"),    tr("Delete selected items"),  "Del",      nullptr, "" )
    initAction( actionCopy_SetlistItem,   ui->eventDatabaseWidget->setlistView(),   tr("&Copy Items"),     tr("Copy selected items"),    "Ctrl+C",   nullptr, "" )
    initAction( actionPaste_SetlistItem,  ui->eventDatabaseWidget->setlistView(),   tr("&Paste Items"),    tr("Paste items"),            "Ctrl+V",   nullptr, "" )

    //////////////////////////////////////////
    ///
    //////////////////////////////////////////
    connect( m_project.songDatabase(), &SongDatabase::attachmentAdded, [this](int i)
    {
        setCurrentAttachment( i );

        AttachmentView* view = ui->songDatabaseWidget->attachmentChooser()->currentAttachmentView();
        if (view && view->inherits("IndexedFileAttachmentView"))
        {
            IndexedFileAttachmentView* ifa = static_cast<IndexedFileAttachmentView*>(view);
            if (ifa->attachment<IndexedFileAttachment>()->hash().isEmpty())
            {
                ifa->chooseFile();
            }
        }
    });
    connect( m_project.songDatabase(), &SongDatabase::attachmentRemoved, [this](int i)
    {
        setCurrentAttachment( i );
    });
    connect( m_project.songDatabase(), &SongDatabase::attachmentRenamed, [this](int i, QString)
    {
        setCurrentAttachment( i );
    });
    connect( m_project.songDatabase(), &SongDatabase::dataChanged, [this]()
    {
        ui->songDatabaseWidget->attachmentChooser()->updateAttachmentView();
    });
    connect( m_project.songDatabase(), &SongDatabase::reseted, [this]()
    {
        ui->songDatabaseWidget->attachmentChooser()->setSong( nullptr );
    });

    //////////////////////////////////////////
    ///
    //////////////////////////////////////////
    connect( &m_project, SIGNAL(canCloseChanged(bool)), this, SLOT(updateWindowTitle()) );
    updateWindowTitle();
    connect( ui->eventDatabaseWidget->databaseView(), SIGNAL(clicked()), this, SLOT(updateActionsEnabled()) );
    connect( ui->eventDatabaseWidget->setlistView(),    SIGNAL(clicked()), this, SLOT(updateActionsEnabled()) );
    connect( ui->songDatabaseWidget->databaseView(),   SIGNAL(clicked()), this, SLOT(updateActionsEnabled()) );
    updateActionsEnabled();


    if (app().arguments().size() > 1 && !app().arguments()[1].startsWith("--"))
    {
        QString filename = app().arguments()[1];
        setCurrentPath(filename);
        open(filename);
    }
    else
    {
        loadDefaultProject();
    }

    connect( &m_project, &Project::songDatabaseCommandPushed, [this]()
    {
        if (m_currentView == EventView)
        {
            activateView(SongView);
        }
    });
    connect( &m_project, &Project::eventDatabaseCommandPushed, [this]()
    {
        activateView(EventView);
    });
    connect( ui->viewSwitchWidget, &SwitchWidget::currentIndexChanged, [this](int index)
    {
        activateView(static_cast<View>(index));
    });
    activateView(static_cast<View>(preference<int>("View")));

    on_actionHide_inactives_triggered(preference<bool>("HideInactived"));
    ui->actionHide_inactives->setChecked(preference<bool>("HideInactived"));

#ifdef ANDROID
    ui->menuBar->hide();
#endif

}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue( "Geometry", saveGeometry() );

    setPreference( "View", static_cast<int>(m_currentView) );
    setPreference( "HideInactived", ui->actionHide_inactives->isChecked() );

    delete ui;
}

Song* MainWindow::currentSong() const
{
    return ui->songDatabaseWidget->currentItem();
}

Event* MainWindow::currentEvent() const
{
    return ui->eventDatabaseWidget->currentItem();
}

void MainWindow::createAttachmentActions()
{
    // gather attachment creators
    for (const QString & classname : Creatable::classnamesInCategory("Attachment"))
    {
        QAction* action = new QAction( this );
        action->setText( QString(tr("New %1")).arg( Creatable::name(classname) ) );

        if (classname == "PDFAttachment")
        {
            action->setIcon(QIcon(":/icons/icons/pdf24.png"));
        }
        else if (classname == "ChordPatternAttachment")
        {
            action->setIcon(QIcon(":/icons/icons/write13.png"));
        }
        else if (classname == "AudioAttachment")
        {
            action->setIcon(QIcon(":/icons/icons/song1.png"));
        }
        else if (classname == "ChordPatternProxyAttachment")
        {
            action->setIcon( QIcon(":/icons/icons/link67.png") );
        }
        else
        {
            qWarning() << QString("action <create %1> has no icon.").arg( classname );
        }

        connect(action, &QAction::triggered, [this, classname]()
        {
            Song* song = currentSong();
            if (song)
            {
                SongNewAttachmentCommand* command = new SongNewAttachmentCommand( song, Creatable::create<Attachment>(classname) );
                pushCommand( command );
                updateActionsEnabled();
            }
        });
        m_newAttachmentActions.insert( classname, action );
    }
}

void MainWindow::setupAttachmentMenu()
{
    // gather attachment creators
    QAction* actionBefore = ui->menuAttachments->actions().first();
    for (const QString & classname : Creatable::classnamesInCategory("Attachment"))
    {
        QAction* action = newAttachment_Action( classname );
        ui->menuAttachments->insertAction(actionBefore, action );
    }
}

void MainWindow::setCurrentAttachment( int index )
{
    ui->songDatabaseWidget->attachmentChooser()->setAttachment( index );
}


//////////////////////////////////////////////////
////
///     Saving and loading stuff
//
////////////////////////////////////////////////


QString MainWindow::projectName() const
{
    QString filename = QFileInfo(m_currentPath).fileName();
    if (filename.isEmpty())
    {
        return tr("Unnamed");
    }
    else
    {
        return filename;
    }
}

bool MainWindow::saveProject()
{
    if (m_currentPath.isEmpty())
    {
        return saveProjectAs();
    }
    else
    {
        QFile file(m_currentPath);
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }
        QDataStream stream(&file);
        stream << m_project;
        m_project.setCanClose(true);


        setCurrentPath( m_currentPath );   // ensure that the current filename is stored as to-default-open project
        updateWindowTitle();
        return true;
    }
}

void MainWindow::updateRecentProjects()
{
    // get list of recent projects, update it and set it.
    QStringList recentProjects = preference<QStringList>("RecentProjects");

    recentProjects.prepend(m_currentPath);

    recentProjects.removeDuplicates();
    while (recentProjects.length() > preference<int>("MaxRecentProjects"))
    {
        recentProjects.removeLast();
    }

    setPreference("RecentProjects", recentProjects);

    // create open recent menu
    if (!ui->actionOpen_recent->menu())
    {
        ui->actionOpen_recent->setMenu( new QMenu(this) );
    }

    ui->actionOpen_recent->menu()->clear();
    for (const QString& filename : recentProjects)
    {
        if (!filename.isEmpty())
        {
            QAction* action = ui->actionOpen_recent->menu()->addAction(filename);
            connect(action, &QAction::triggered, [filename, this, action]()
            {
                if (!this->open(filename))
                {
                    QStringList recentProjects = preference<QStringList>("RecentProjects");
                    recentProjects.removeAll(filename);
                    setPreference("RecentProjects", recentProjects);
                    updateRecentProjects();
                }
            });
        }
    }
}

void MainWindow::setCurrentPath(const QString &path)
{
    m_currentPath = path;

    updateRecentProjects();
}

QString MainWindow::proposedPath() const
{
    QString filename = projectName();
    QString ending = QString(".%1").arg(m_project.ending());

    if (!filename.endsWith(ending))
    {
        filename = filename.append(ending);
    }

    if (m_currentPath.isEmpty())
    {
        return QDir::home().absoluteFilePath( filename );
    }
    else
    {
        return m_currentPath;
    }
}

bool MainWindow::saveProjectAs()
{

    QString filename =
    QFileDialog::getSaveFileName( this,
                                  tr("Save As ..."),
                                  proposedPath(),
                                  Application::PROJECT_FILE_FILTER,
                                  nullptr,
                                  QFileDialog::DontConfirmOverwrite ); // we implement own overwrite-check below.
    if (filename.isEmpty())
    {
        return false;
    }
    else
    {
        QString ending = "." + m_project.ending();
        if (!filename.endsWith(ending))
        {
            filename.append(ending);
        }

        if (QFileInfo(filename).exists())
        {
            QMessageBox::Button result =
            QMessageBox::warning( this,
                                  tr("Save as ..."),
                                  tr("%1 already exists.\nDo you want to replace it?").arg(QFileInfo(filename).fileName()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No );
            if (result != QMessageBox::Yes)
            {
                // begin from start
                return saveProjectAs();
            }
        }
        // if  filename  does not exist or if we have permission to overwrite
        setCurrentPath(filename);
        return saveProject();
    }
}

bool MainWindow::newProject()
{
    if (!canProjectClose())
    {
        return false;
    }

    m_project.reset();
    setCurrentPath( QString() );
    updateWindowTitle();
    updateActionsEnabled();
    return true;
}

bool MainWindow::canProjectClose()
{
    if (m_project.canClose())
    {
        return true;
    }
    else
    {
        switch (
        QMessageBox::question( this,
                               tr("Really close %1").arg( projectName() ),
                               tr("There are unsaved changes."),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Abort,
                               QMessageBox::Abort ) )
        {
        case QMessageBox::Save:
            return saveProject();
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Abort:
            return false;
        default:
            WARNING << "Illegal case in switch statement";
            return false;
        }
    }
}

void MainWindow::updateWindowTitle()
{
    QString star = m_project.canClose() ? "" : "*";

    QString title = QString("%1%2 - %4")
            .arg( projectName() )
            .arg( star )
            .arg( app().applicationName() );

    setWindowTitle( title );
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (canProjectClose())
    {
        QMainWindow::closeEvent(e);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void MainWindow::loadDefaultProject()
{
    QStringList recentProjects = preference<QStringList>("RecentProjects");

    if (recentProjects.isEmpty())
    {
        setCurrentPath("");
    }
    else
    {
        QString filename = recentProjects.first();
        if (filename.isEmpty() || !open(filename))
        {
            setCurrentPath("");
        }
        else
        {
            setCurrentPath(filename);
        }
    }
}

int MainWindow::currentAttachmentIndex() const
{
    return ui->songDatabaseWidget->attachmentChooser()->currentAttachmentIndex();
}

Attachment* MainWindow::currentAttachment() const
{
    return ui->songDatabaseWidget->attachmentChooser()->currentAttachment();
}

void setEnabled( QObject* o, bool enable )
{
    QAction* a = qobject_cast<QAction*>(o);
    if (a) a->setEnabled(enable);

    QWidget* w = qobject_cast<QWidget*>(o);
    if (w) w->setEnabled(enable);
}

void MainWindow::updateActionsEnabled()
{
    bool song = !!currentSong();
    bool event = !!currentEvent();
    bool attachment = !!currentAttachment();
    bool setlist = !ui->eventDatabaseWidget->setlistView()->selectionModel()->selectedRows().isEmpty();

    QList<QAction*> attachmentActions, songActions, eventActions, setlistActions;

    for (QAction* action : m_newAttachmentActions)
    {
        songActions << action;
    }

    attachmentActions   << ui->actionDuplicate_Attachment << ui->actionRename_Attachment << ui->actionDelete_Attachment;
    songActions         << m_actionDelete_Song << m_actionCopy_Song;
#ifdef HAVE_PROGRAM
    songActions << m_actionEdit_Program;
#endif

    for (QAction* o : eventActions )
    {
        o->setEnabled(event);
    }
    for (QAction* o : songActions )
    {
        o->setEnabled(song);
    }
    for (QAction* o : attachmentActions )
    {
        o->setEnabled(attachment);
    }
    for (QAction* o : setlistActions )
    {
        o->setEnabled(setlist);
    }

    bool chordPatternProxyAttachmentEnabled = false;
    if (currentAttachment() && currentAttachment()->inherits( "ChordPatternAttachment" ))
    {
        chordPatternProxyAttachmentEnabled = true;
    }
    m_newAttachmentActions["ChordPatternProxyAttachment"]->setEnabled( chordPatternProxyAttachmentEnabled );

    {
        const QMimeData* clipboard = app().clipboard()->mimeData();
        m_actionPaste_Event->setEnabled( clipboard->hasFormat(DatabaseMimeData<Event>::mimeType()) );
        m_actionPaste_SetlistItem->setEnabled( clipboard->hasFormat(DatabaseMimeData<SetlistItem>::mimeType())) ;
        m_actionPaste_Song->setEnabled( clipboard->hasFormat(DatabaseMimeData<Song>::mimeType()) );
    }

    ui->actionSync->setEnabled(m_project.remoteInfo().isValid());

}

////////////////////////////////////////////////
////
///     Connections to Actions
//
///////////////////////////////////////////////


void MainWindow::my_on_actionNew_Song_triggered()
{
    Song* song =  new Song(m_project.songDatabase());
    pushCommand( new DatabaseNewItemCommand<Song>( m_project.songDatabase(), song)  );

    // add default chord pattern attachment
    Attachment* attachment = Creatable::create<Attachment>("ChordPatternAttachment");
    attachment->setSong(song);
    song->addAttachment( attachment );

    updateActionsEnabled();
}

void MainWindow::on_actionUndo_triggered()
{
    m_project.undo();
    updateActionsEnabled();
}

void MainWindow::on_actionRedo_triggered()
{
    m_project.redo();
    updateActionsEnabled();
}

void MainWindow::on_actionDelete_Attachment_triggered()
{
    Song* song = currentSong();
    int index = ui->songDatabaseWidget->attachmentChooser()->currentAttachmentIndex();

    if (song && index >= 0)
    {
        pushCommand( new SongRemoveAttachmentCommand( song, index ) );
        updateActionsEnabled();
    }
}

void MainWindow::on_actionNew_Project_triggered()
{
    newProject();
    updateActionsEnabled();
}

void MainWindow::on_actionSave_triggered()
{
    saveProject();
    updateActionsEnabled();
}

void MainWindow::on_actionSave_As_triggered()
{
    saveProjectAs();
    updateActionsEnabled();
}

void MainWindow::on_actionOpen_triggered()
{
    if (!canProjectClose())
    {
        return; // user aborted opening;
    }

    QString filename =
    QFileDialog::getOpenFileName( this,
                                  tr("Open ..."),
                                  proposedPath(),
                                  Application::PROJECT_FILE_FILTER           );
    if (filename.isEmpty())
    {
        return; // user aborted opening
    }
    else
    {
        open(filename);
    }
}

void MainWindow::my_on_actionDelete_Song_triggered()
{
    Song* song = currentSong();
    if (song)
    {
        if (song->canRemove())
        {
            pushCommand( new DatabaseRemoveItemCommand<Song>( m_project.songDatabase(), song ) );
        }
        else
        {
            QMessageBox::warning( this,
                                  tr("Song cannot be removed"),
                                  tr("This song is currently in use and can thus not be removed."),
                                  QMessageBox::Ok,
                                  QMessageBox::NoButton );
        }
        updateActionsEnabled();
    }
}

void MainWindow::on_actionAdd_Folder_triggered()
{
    enum { ShowNotOnlyDirs = 0x0 };
    QString path = QFileDialog::getExistingDirectory(this, tr("Add to index ..."), preference<QString>("FileIndexDefaultPath"));

    if (path.isEmpty())
    {
        return;
    }

    setPreference("FileIndexDefaultPath", path);

    QProgressDialog pd( "Task in Progress", "Cancel", 0, -1, this );
    pd.setWindowModality( Qt::WindowModal );

    app().fileIndex()->addDirectory( path );

    QLabel* label = new QLabel(&pd);
    label->setWordWrap(true);
    pd.setLabel(label);
    pd.show();

    while (!app().fileIndex()->operationIsFinished())
    {
        pd.setValue( (pd.value() + 1) % 100 );

//        qDebug() << label;
//        qDebug() << &app();
//        qDebug() << app().fileIndex();
//        qDebug() << app().fileIndex()->currentFilename();

        label->setText( QString("%1\n%2").arg(app().fileIndex()->currentFilename())
                                         .arg(app().fileIndex()->size())            );
        qApp->processEvents();
        QThread::msleep( 10 );
        if (pd.wasCanceled())
        {
            app().fileIndex()->abortOperations();
        }
    }

}

void MainWindow::on_actionClear_Index_triggered()
{
    app().fileIndex()->clear();
}

void MainWindow::on_actionRename_Attachment_triggered()
{
    ui->songDatabaseWidget->attachmentChooser()->renameCurrentAttachment();
}

void MainWindow::on_actionDuplicate_Attachment_triggered()
{
    Song* cs = currentSong();
    Q_ASSERT( cs );

    int index = ui->songDatabaseWidget->attachmentChooser()->currentAttachmentIndex();
    Q_ASSERT( index >= 0 );

    Attachment* attachment = cs->attachments()[index];
    Q_ASSERT( attachment );

    pushCommand( new SongNewAttachmentCommand( cs, attachment->copy() ) );
    updateActionsEnabled();
}


void MainWindow::on_actionSettings_triggered()
{
    PreferenceDialog dialog(app().preferences(), this);
    dialog.exec();
}

void MainWindow::on_action_Index_Info_triggered()
{
    QMessageBox::information( this,
                              tr("File index information"),
                              QString(tr("Files: %1")).arg(app().fileIndex()->size()) );
}

void MainWindow::my_on_actionNew_Event_triggered()
{
    Event* newEvent = new Event(m_project.eventDatabase());
    pushCommand( new DatabaseNewItemCommand<Event>( m_project.eventDatabase(), newEvent) );
    static_cast<EventTableView*>(ui->eventDatabaseWidget->databaseView())->triggerEditDate(newEvent);
}

void MainWindow::my_on_actionDelete_Event_triggered()
{
    Event* event = currentEvent();
    if (event)
    {
        pushCommand( new DatabaseRemoveItemCommand<Event>( m_project.eventDatabase(), event ));
        updateActionsEnabled();
    }
}


QAction* MainWindow::undoAction() const
{
    return ui->actionUndo;
}

QAction* MainWindow::redoAction() const
{
    return ui->actionRedo;
}

QAction* MainWindow::newAttachment_Action( const QString& classname )
{
    if (m_newAttachmentActions.contains(classname))
    {
        return m_newAttachmentActions[classname];
    }
    else
    {
        qWarning() << "Attachment " << classname << "not known." << m_newAttachmentActions.keys();
        return nullptr;
    }
}

void MainWindow::my_on_actionCopy_Song_triggered()
{
    QModelIndexList selectedSongs = ui->songDatabaseWidget->databaseView()->selectionModel()->selectedIndexes();

    for (QModelIndex& index : selectedSongs)
    {
        index = ui->songDatabaseWidget->databaseView()->sortProxy()->mapToSource(index);
    }
    app().clipboard()->setMimeData( m_project.songDatabase()->mimeData( selectedSongs ) );
}

void MainWindow::my_on_actionPaste_Song_triggered()
{
    app().project()->songDatabase()->dropMimeData(app().clipboard()->mimeData(), Qt::CopyAction, m_project.songDatabase()->rowCount(), 0, QModelIndex());
}

void MainWindow::my_on_actionCopy_Event_triggered()
{
    QModelIndexList selectedEvents = ui->eventDatabaseWidget->databaseView()->selectionModel()->selectedIndexes();
    for (QModelIndex& index : selectedEvents)
    {
        index = ui->eventDatabaseWidget->databaseView()->proxyModel()->mapToSource(index);
    }
    app().clipboard()->setMimeData( m_project.eventDatabase()->mimeData( selectedEvents ) );
}

void MainWindow::my_on_actionPaste_Event_triggered()
{
    app().project()->eventDatabase()->dropMimeData(app().clipboard()->mimeData(), Qt::CopyAction, m_project.eventDatabase()->rowCount(), 0, QModelIndex());
}

void MainWindow::my_on_actionEdit_Program_triggered()
{
#ifdef HAVE_PROGRAM
    if (!currentSong())
    {
        return;
    }

    ProgramDialog pd(this);
    pd.setProgram( currentSong()->program() );
    if (pd.exec() == QDialog::Accepted)
    {
        pushCommand( new SongEditProgramCommand( currentSong(), pd.program() ) );
    }
#endif
}

bool MainWindow::open(const QString &filename)
{
    bool success;
    OpenError error = m_project.openProject(filename);
    if (error == NoError)
    {
        success = true;
        setCurrentPath(filename);
    }
    else
    {
        success = false;
        m_project.reset();
        setCurrentPath("");
        app().handleProjectOpenError(error, filename);
    }
    updateWindowTitle();
    updateActionsEnabled();

    return success;
}

void MainWindow::createLanguageMenu()
{
    QActionGroup* languageGroup = new QActionGroup(this);
    for (const QFileInfo& fileInfo : QDir(":/translations/").entryInfoList(QStringList() << "can*.qm", QDir::Files, QDir::Name ))
    {
        QString locale = fileInfo.baseName().mid(4); // skip ending, skip can_
        QAction* action = ui->menu_Language->addAction( QLocale(locale).nativeLanguageName() );
        action->setCheckable( true );

        if (preference<QString>("locale") == locale)
        {
            action->setChecked(true);
        }

        connect( action, &QAction::triggered, [this, locale]()
        {
            QMessageBox::information( this,
                                      tr("Information"),
                                      tr("Language changes will apply on next start."),
                                      QMessageBox::Ok,
                                      QMessageBox::NoButton );

            setPreference("locale", locale);
        });
        languageGroup->addAction(action);
    }
    languageGroup->setExclusive(true);
}

DatabaseView<Song>* MainWindow::songTableView()
{
    return ui->songDatabaseWidget->databaseView();
}


void MainWindow::on_action_Export_all_songs_triggered()
{
    OrphantSetlist setlist( tr("All songs"));

    for( Song* song : m_project.songDatabase()->items() )
    {
        setlist.insertItem( new SetlistItem( &setlist, song ) );
    }

    PDFCreator::exportSetlist( &setlist, this );
}

void MainWindow::my_on_actionNew_SetlistItem_triggered()
{
    if (currentEvent())
    {
        Setlist* setlist = currentEvent()->setlist();
        pushCommand( new DatabaseNewItemCommand<SetlistItem>( setlist, new SetlistItem(setlist) ) );
    }
}

void MainWindow::my_on_actionDelete_SetlistItem_triggered()
{
    QList<SetlistItem*> si = ui->eventDatabaseWidget->setlistView()->selectedItems();
    if (currentEvent() && !si.isEmpty())
    {
        Setlist* setlist = currentEvent()->setlist();
        app().project()->beginMacro( tr("Remove Setlist Items"));
        for (SetlistItem* i : si)
        {
            pushCommand( new DatabaseRemoveItemCommand<SetlistItem>( setlist, i ) );
        }
        app().project()->endMacro();
    }
}

void MainWindow::my_on_actionCopy_SetlistItem_triggered()
{
    if (currentEvent())
    {
        QModelIndexList selection = ui->eventDatabaseWidget->setlistView()->selectionModel()->selectedRows();
        app().clipboard()->setMimeData( currentEvent()->setlist()->mimeData( selection ) );
    }
}

void MainWindow::my_on_actionPaste_SetlistItem_triggered()
{
    if (currentEvent())
    {
        Setlist* setlist = currentEvent()->setlist();
        int row = setlist->rowCount();
        QModelIndexList selection = ui->eventDatabaseWidget->setlistView()->selectionModel()->selectedRows();
        if (!selection.isEmpty())
        {
            row = selection.last().row() + 1;
        }
        setlist->dropMimeData(app().clipboard()->mimeData(), Qt::CopyAction, row, 0, QModelIndex());
    }
}

void MainWindow::on_actionCopy_Indexed_Attachments_triggered()
{
    CopyIndexedFilesDialog dialog(app().project()->songDatabase()->items());

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList filenames = dialog.sources();

        if (filenames.isEmpty())
        {
            QMessageBox::information(this, tr("Nothing copied"), tr("You have not selected any file."), QMessageBox::Ok);
        }
        else
        {
            QString path = QFileDialog::getExistingDirectory(this, Application::applicationName(), preference<QString>("RecentCopyIndexedFilesTargetDir"));
            if (!path.isEmpty())
            {
                setPreference("RecentCopyIndexedFilesTargetDir", path);
                QDir dir(path);

                QStringList success, failure, notOverwritten;

                for (const QString & filename : filenames)
                {
                    QString newName = dir.absoluteFilePath(QFileInfo(filename).fileName());

                    if (QFile::copy(filename, newName))
                    {
                        success << newName;
                    }
                    else
                    {
                        if (QFileInfo(newName).exists())
                        {
                            notOverwritten << newName;
                        }
                        else
                        {
                            failure << newName;
                        }
                    }
                }

                QMessageBox::information( this,
                                          Application::applicationName(),
                                          tr("Copying finished.\n  Successfull:\t%1\n  Not overwritten:\t%2\n  Failure:\t\t%3")
                                            .arg(success.count())
                                            .arg(notOverwritten.count())
                                            .arg(failure.count()) );
            }
        }
    }
}

void MainWindow::on_actionEvents_triggered()
{
    activateView(EventView);
}

void MainWindow::on_actionSongs_triggered()
{
    activateView(SongView);
}

void MainWindow::on_actionSong_Details_triggered()
{
    activateView(DetailedSongView);
}

void MainWindow::activateView(View view)
{
    m_currentView = view;
    if (view == SongView)
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->actionSongs->setChecked(true);
        ui->viewSwitchWidget->setCurrentIndex(0);
        static_cast<SongTableView*>(ui->songDatabaseWidget->databaseView())->setDetailedView(false);
    }
    else if (view ==  DetailedSongView)
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->actionSong_Details->setChecked(true);
        ui->viewSwitchWidget->setCurrentIndex(1);
        static_cast<SongTableView*>(ui->songDatabaseWidget->databaseView())->setDetailedView(true);

    }
    else if (view == EventView)
    {
        ui->actionEvents->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
        ui->viewSwitchWidget->setCurrentIndex(2);
    }
}

void MainWindow::on_actionHide_inactives_triggered(bool checked)
{
    static_cast<SongTableView*>(ui->songDatabaseWidget->databaseView())->setHideInactives(checked);
}

void MainWindow::on_actionMerge_with_triggered()
{
    if (!m_project.canClose())
    {
        // saving is required
        if (!saveProject())
        {
            // saving failed
            return;
        }
        // saving succeeded
    }

    QString filename =
    QFileDialog::getOpenFileName( this,
                                  tr("Open ..."),
                                  proposedPath(),
                                  Application::PROJECT_FILE_FILTER  );
    if (filename.isEmpty())
    {
        return; // user aborted opening
    }
    else if ( !MergeDialog::performMerge(&m_project, filename, this) )
    {
        return; // user aborted merging
    }
    else
    {
        // merge was done.
        m_project.setCanClose(false);
        m_project.QUndoStack::clear();
    }
}

void MainWindow::on_actionSync_triggered()
{
#ifdef HAVE_LIBGIT
    if (!m_project.canClose() || m_currentPath.isEmpty())
    {
        if (QMessageBox::question(this, qAppName(), tr("Projects must be saved before synchronizing.\nSave Project?"), QMessageBox::Yes, QMessageBox::Abort) == QMessageBox::Yes)
        {
            saveProject();
        }
        else
        {
            return;
        }
    }

    GitHandler git;
    RemoteInfo remote = m_project.remoteInfo();

    Q_ASSERT(remote.isValid());

    if (GitDialog::sync(&git, remote.url(), remote.path(), m_currentPath, &m_project, this))
    {
        open(m_currentPath);    //reopen file
    }
#endif
}

void MainWindow::on_actionOpen_cloud_file_triggered()
{
#ifdef HAVE_LIBGIT
    GitHandler git;
    QString filename, url, saveFilename;
    if (GitDialog::download(&git, url, filename, saveFilename, this))
    {
        if (!open(saveFilename))
        {
            QMessageBox::warning(this, qAppName(), tr("Could not open downloaded file: %1").arg(saveFilename));
            newProject();
        }
        else
        {
            m_project.setRemoteInfo(url, filename);
        }
    }
#endif
}

#include <QTextEdit>
#include "tojson.h"
void MainWindow::on_actionExport_Json_triggered()
{
    QDialog* dialog = new QDialog(this);
    dialog->setLayout(new QHBoxLayout(dialog));
    QTextEdit* edit = new QTextEdit(this);
    dialog->layout()->addWidget(edit);
    edit->setText(ToJson(&m_project).toString());
    dialog->exec();
}
