#include "project.h"

#include <QCryptographicHash>
#include <QFile>

#include "Commands/command.h"
#include "global.h"
#include "Database/SongDatabase/songdatabase.h"
#include "Database/EventDatabase/eventdatabase.h"
#include "Database/EventDatabase/event.h"


const QByteArray Project::SERIALIZE_KEY = "f24e693129d1b378d3496a42cb7096bd";

Project::Project() :
    m_songDatabase( new SongDatabase(this) ),
    m_eventDatabase( new EventDatabase(this) )
{
    reset();
}

Project::~Project()
{
    delete m_songDatabase;
    delete m_eventDatabase;
}

void Project::setCommandFocalizesAffiliatedView(bool on)
{
    m_commandFocalizesAffiliatedView = on;
}

void Project::setCanClose(bool b)
{
    if (m_canClose != b)
    {
        m_canClose = b;
        emit canCloseChanged(b);
    }
}


void emitCommandPushedSignal(Command::Type type, Project* project, bool commandFocalizesAffiliatedView)
{
    switch (type)
    {
    case Command::SongDatabaseRelated:
        if (commandFocalizesAffiliatedView)
        {
            emit project->songDatabaseCommandPushed();
        }
        break;
    case Command::EventDatabaseRelated:
        if (commandFocalizesAffiliatedView)
        {
            emit project->eventDatabaseCommandPushed();
        }
        break;
    case Command::Other:
        ;
    }
    emit project->commandPushed();
}

void Project::pushCommand(Command *command)
{
    setCanClose(false);

    if (command)
    {
        QUndoStack::push(command);
        emitCommandPushedSignal( command->type(), this, m_commandFocalizesAffiliatedView );
    }

}

void Project::undo()
{
    const QUndoCommand* uc = QUndoStack::command(QUndoStack::index() - 1);
    const Command* c = dynamic_cast<const Command*>( uc );
    QUndoStack::undo();

    if (c)  // cast may fail (e.g. macro command)
    {
        emitCommandPushedSignal( c->type(), this, true );
    }

    setCanClose( false );

}

void Project::redo()
{
    const QUndoCommand* uc = QUndoStack::command(QUndoStack::index());
    const Command* c = dynamic_cast<const Command*>( uc );
    QUndoStack::redo();

    if (c)  // cast may fail (e.g. command-macro)
    {
        emitCommandPushedSignal( c->type(), this, true );
    }

    setCanClose( false );

}

void Project::reset()
{
    m_songDatabase->reset();
    m_eventDatabase->reset();
    QUndoStack::clear();
    setCanClose(true);
    emit undoStackCleared();
}

bool Project::canClose() const
{
    return m_canClose;
}

QByteArray hash(const QByteArray& data)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data);
    return hash.result();
}

QDataStream& operator<<(QDataStream& out, const Project& project)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << project.m_songDatabase;
    stream << project.m_eventDatabase;
    stream << project.m_remoteInfo;

    // stream KEY
    out << Project::SERIALIZE_KEY;

    // stream hash value of data
    out << hash(data);

    // stream data itself
    out << data;


    return out;
}

QDataStream& operator>>(QDataStream& in, Project& project)
{
    QByteArray key, hash, data;
    in >> key >> hash >> data;

    // we do the check in a separate function to obtain more flexibility
    Q_UNUSED(key);
    Q_UNUSED(hash);

    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> project.m_songDatabase;
    stream >> project.m_eventDatabase;

    if (!stream.atEnd()) //TODO  remove legacy check
    {
        stream >> project.m_remoteInfo;
    }
    else
    {
    }

    project.setCanClose(true);

    return in;
}

OpenError Project::openProject(const QString& filename)
{
    // try to open the file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        return CannotReadFileError;
    }

    QByteArray data = file.readAll();

    // try to read the file
    {
    QDataStream stream(data);
    QByteArray key, projectHash, projectData;
    stream >> key >> projectHash >> projectData;

    if (key != Project::SERIALIZE_KEY)
    {
        return InvalidFileFormatError;
    }
    else if (hash(projectData) != projectHash)
    {
        return InvalidFileFormatError;
    }
    else
    {
        QDataStream stream(data);
        stream >> *this;

        return NoError;
    }
    }
}

RemoteInfo Project::remoteInfo() const
{
    return m_remoteInfo;
}

void Project::setRemoteInfo(const QString &url, const QString &filename)
{
    m_remoteInfo = RemoteInfo(url, filename);
}
