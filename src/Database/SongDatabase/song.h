#ifndef SONG_H
#define SONG_H

#include <QList>
#include <QDateTime>

#include "Program/program.h"
#include "Database/databaseitem.h"
#include "chord.h"

class SongDatabase;
class Attachment;
template<typename T> class Database;
class Song : public DatabaseItem<Song>
{
    Q_OBJECT
public:
    Song(Database<Song> *database);
    ~Song();

    enum Label { NoLabel, Acoustic, Normal, AcousticNormal };
    enum State { NoState, Inactive, NeedsPractice, Works };

    /////////////////////////////////////////////////
    ////
    ///  Attributes
    //
    /////////////////////////////////////////////////
public:
    static const QStringList ATTRIBUTE_KEYS;
    QString attributeDisplay(const QString& key) const;


    /////////////////////////////////////////////////
    ////
    ///  Attachments
    //
    /////////////////////////////////////////////////
private:
    QList<Attachment*> m_attachments;
    void connectAttachment(Attachment* attachment);
public:
    QList<Attachment*> attachments() const { return m_attachments; }
    Attachment* attachment( int i ) const { return m_attachments[i]; }
    QStringList attachmentNames() const;
    int removeAttachment(Attachment *attachment );
    void addAttachment(Attachment *attachment );
    void insertAttachment(Attachment* attachment, int index);


signals:
    void attachmentAdded(int i);
    void attachmentRemoved(int i);
    void attachmentRenamed(int i, QString);
    void attributeChanged();

    /////////////////////////////////////////////////
    ////
    ///  Programs
    //
    /////////////////////////////////////////////////
public:
    Program program() const { return m_program; }
    void setProgram( const Program& program ) { m_program = program; }
private:
    Program m_program = Program();

protected:
    void serialize(QDataStream &out) const;
    void deserialize(QDataStream &in);


public:
    static QStringList stateNames();
    static QStringList labelNames();

};

Q_DECLARE_METATYPE(const Song*)
Q_DECLARE_METATYPE(Song::Label)
Q_DECLARE_METATYPE(Song::State)
REGISTER_META_TYPE_STREAM_OPERATORS(Song::Label, SongLabel)
REGISTER_META_TYPE_STREAM_OPERATORS(Song::State, SongState)
QDataStream& operator <<(QDataStream& out, const Song::State& state);
QDataStream& operator >>(QDataStream& in,        Song::State& state);
QDataStream& operator <<(QDataStream& out, const Song::Label& state);
QDataStream& operator >>(QDataStream& in,        Song::Label& state);

#endif // SONG_H
