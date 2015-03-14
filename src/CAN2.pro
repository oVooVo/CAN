#-------------------------------------------------
#
# Project created by QtCreator 2015-01-22T22:41:48
#
#-------------------------------------------------

QT       += core gui multimedia

QT += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CAN2
TEMPLATE = app

LIBS += -L../../build-ZipGit-Desktop-Debug/ -lzipgit
LIBS += -lgit2
LIBS += -L/usr/local/lib -lpoppler-qt5


QMAKE_CXXFLAGS += -std=c++0x


SOURCES += main.cpp\
        mainwindow.cpp \
    configurable.cpp \
    project.cpp \
    Database/SongDatabase/songdatabase.cpp \
    Database/DateDatabase/datedatabase.cpp \
    Database/database.cpp \
    taggable.cpp \
    Database/SongDatabase/song.cpp \
    Attachments/attachment.cpp \
    persistentobject.cpp \
    creatable.cpp \
    Attachments/ChordPatternAttachment/chordpatternattachment.cpp \
    Database/DateDatabase/date.cpp \
    UnitTest/creatabletest.cpp \
    UnitTest/unittests.cpp \
    Commands/command.cpp \
    Commands/SongDatabaseCommands/songdatabasesetdatacommand.cpp \
    Commands/SongDatabaseCommands/songdatabasenewsongcommand.cpp \
    SongTableView/songtableview.cpp \
    Commands/SongDatabaseCommands/songdatabaseeditsongcommand.cpp \
    Commands/SongDatabaseCommands/songdatabasecommand.cpp \
    Commands/SongDatabaseCommands/songdatabaseremovesongcommand.cpp \
    Commands/SongDatabaseCommands/songdatabasenewattributecommand.cpp \
    SongTableView/renamableheaderview.cpp \
    Commands/SongDatabaseCommands/songdatabaserenameheadercommand.cpp \
    Commands/SongDatabaseCommands/songdatabaseremovecolumncommand.cpp \
    util.cpp \
    SongTableView/songattributedelegate.cpp \
    SongTableView/CellEditors/celleditor.cpp \
    SongTableView/CellEditors/stringeditor.cpp \
    SongTableView/CellEditors/comboeditor.cpp \
    SongTableView/CellEditors/dateeditor.cpp \
    Database/SongDatabase/songdatabasesortproxy.cpp \
    SongTableView/songtableviewcontainer.cpp \
    filterwidget.cpp \
    progressdialog.cpp \
    FileIndex/fileindex.cpp \
    FileIndex/indexer.cpp \
    Attachments/indexedfileattachment.cpp \
    application.cpp \
    Commands/SongCommands/songaddattachmentcommand.cpp \
    Commands/SongCommands/songcommand.cpp \
    Commands/SongCommands/songremoveattachmentcommand.cpp \
    Commands/AttachmentCommands/fileattachmentcommandmodifyfilenamecommand.cpp \
    AttachmentView/attachmenteditor.cpp \
    AttachmentView/attachmentview.cpp \
    AttachmentView/chordpatternattachmentview.cpp \
    AttachmentView/pdfattachmentview.cpp \
    Attachments/pdfattachment.cpp \
    songdatabasewidget.cpp \
    AttachmentView/attachmentchooser.cpp \
    Commands/AttachmentCommands/attachmentcommand.cpp \
    Commands/AttachmentCommands/attachmentrenamecommand.cpp \
    Attachments/ChordPatternAttachment/chord.cpp \
    Attachments/ChordPatternAttachment/chordpattern.cpp \
    Dialogs/clonedialog.cpp \
    Dialogs/pushdialog.cpp \
    Dialogs/pulldialog.cpp \
    Commands/AttachmentCommands/chordpatternattachmenttransposecommand.cpp \
    Dialogs/addfileindexsourcedialog.cpp \
    stringdialog.cpp \
    Commands/SongCommands/songduplicateattachmentcommand.cpp \
    AttachmentView/indexedfileattachmentview.cpp \
    AttachmentView/audioattachmentview.cpp \
    Attachments/audioattachment.cpp \
    AttachmentView/chordpatternedit.cpp \
    advancedfilechooser.cpp \
    conflicteditor.cpp \
    advancedaudioplayer.cpp

HEADERS  += mainwindow.h \
    project.h \
    configurable.h \
    Database/SongDatabase/songdatabase.h \
    Database/DateDatabase/datedatabase.h \
    Database/database.h \
    taggable.h \
    Database/SongDatabase/song.h \
    Attachments/attachment.h \
    persistentobject.h \
    creatable.h \
    Attachments/ChordPatternAttachment/chordpatternattachment.h \
    Database/DateDatabase/date.h \
    UnitTest/creatabletest.h \
    UnitTest/unittests.h \
    Commands/command.h \
    Commands/SongDatabaseCommands/songdatabasesetdatacommand.h \
    Commands/SongDatabaseCommands/songdatabasenewsongcommand.h \
    SongTableView/songtableview.h \
    Commands/SongDatabaseCommands/songdatabaseeditsongcommand.h \
    Commands/SongDatabaseCommands/songdatabasecommand.h \
    Commands/SongDatabaseCommands/songdatabaseremovesongcommand.h \
    Commands/SongDatabaseCommands/songdatabasenewattributecommand.h \
    SongTableView/renamableheaderview.h \
    Commands/SongDatabaseCommands/songdatabaserenameheadercommand.h \
    Commands/SongDatabaseCommands/songdatabaseremovecolumncommand.h \
    util.h \
    SongTableView/songattributedelegate.h \
    SongTableView/CellEditors/celleditor.h \
    SongTableView/CellEditors/stringeditor.h \
    SongTableView/CellEditors/comboeditor.h \
    SongTableView/CellEditors/dateeditor.h \
    Database/SongDatabase/songdatabasesortproxy.h \
    SongTableView/songtableviewcontainer.h \
    filterwidget.h \
    progressdialog.h \
    FileIndex/fileindex.h \
    FileIndex/indexer.h \
    Attachments/indexedfileattachment.h \
    application.h \
    Commands/SongCommands/songaddattachmentcommand.h \
    Commands/SongCommands/songcommand.h \
    Commands/SongCommands/songremoveattachmentcommand.h \
    Commands/AttachmentCommands/fileattachmentcommandmodifyfilenamecommand.h \
    Commands/AttachmentCommands/attachmentrenamecommand.h \
    AttachmentView/attachmenteditor.h \
    AttachmentView/attachmentview.h \
    AttachmentView/chordpatternattachmentview.h \
    AttachmentView/pdfattachmentview.h \
    Attachments/pdfattachment.h \
    songdatabasewidget.h \
    AttachmentView/attachmentchooser.h \
    Commands/AttachmentCommands/attachmentcommand.h \
    Attachments/ChordPatternAttachment/chord.h \
    Attachments/ChordPatternAttachment/chordpattern.h \
    Dialogs/clonedialog.h \
    Dialogs/pushdialog.h \
    Dialogs/pulldialog.h \
    Commands/AttachmentCommands/chordpatternattachmenttransposecommand.h \
    Dialogs/addfileindexsourcedialog.h \
    stringdialog.h \
    Commands/SongCommands/songduplicateattachmentcommand.h \
    AttachmentView/indexedfileattachmentview.h \
    AttachmentView/audioattachmentview.h \
    Attachments/audioattachment.h \
    AttachmentView/chordpatternedit.h \
    advancedfilechooser.h \
    conflicteditor.h \
    advancedaudioplayer.h \
    global.h

FORMS    += mainwindow.ui \
    SongTableView/songtableviewcontainer.ui \
    filterwidget.ui \
    progressdialog.ui \
    AttachmentView/chordpatternattachmentview.ui \
    AttachmentView/pdfattachmentview.ui \
    songdatabasewidget.ui \
    AttachmentView/attachmentchooser.ui \
    Dialogs/clonedialog.ui \
    Dialogs/pushdialog.ui \
    Dialogs/pulldialog.ui \
    stringdialog.ui \
    AttachmentView/indexedfileattachmentview.ui \
    advancedfilechooser.ui \
    AttachmentView/audioattachmentview.ui \
    conflicteditor.ui

OTHER_FILES += \
    FileIndexRecycle.txt

RESOURCES += \
    ressources.qrc
