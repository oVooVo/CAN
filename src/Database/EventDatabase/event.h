#ifndef DATE_H
#define DATE_H

#include <QJsonObject>
#include <QDateTime>
#include <QObject>
#include "global.h"

#include "Database/databaseitem.h"

class Setlist;
class Song;
template<typename T> class Database;
class Event : public DatabaseItem<Event>
{
    Q_OBJECT
public:
    enum Type { Rehearsal, Gig, Other };


    Event(Database<Event>*   database,
           const QDateTime& beginning = QDateTime::currentDateTime(),
           Type             type      = Rehearsal,
           const QString &  label = "");
    ~Event();

    QString description() const;

    const Setlist* setlist() const { return m_setlist; }
    void setSetlist(const Setlist *newSetlist);
    Setlist* setlist() { return m_setlist; }

    QString attributeDisplay(const QString &key) const;

    static QStringList eventTypeNames();
    static QString eventTypeName(Type type);

    QString label() const;

    bool needsSong(const Song* song) const;

protected:
    void serialize(QDataStream &out) const;
    void deserialize(QDataStream &in);

private:
    Setlist* m_setlist;

public:
    Ratio similarity(const DatabaseItemBase *other) const;

};

Q_DECLARE_METATYPE(Event::Type)

DECLARE_ENUM_STREAM_OPERATORS(Event::Type)


#endif // DATE_H
