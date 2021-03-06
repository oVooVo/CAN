#ifndef SETLISTITEM_H
#define SETLISTITEM_H

#include <QJsonObject>
#include "Database/databaseitem.h"
#include "Database/SongDatabase/song.h"

class Song;
class Event;
class Setlist;
template<typename T> class Database;
class SetlistItem : public DatabaseItem<SetlistItem>
{
    Q_OBJECT
public:
    enum Type { SongType, LabelType };
public:
    SetlistItem(Database<SetlistItem> *setlist, const QString & label, const Song* song );
    SetlistItem(Database<SetlistItem> *setlist, const QString & label );
    SetlistItem(Database<SetlistItem> *setlist );
    SetlistItem(Database<SetlistItem> *setlist, const Song* song );
    virtual ~SetlistItem();

    void setSong(const Song* song);

    QString attributeDisplay(const QString &key) const;

    static const QStringList ATTRIBUTE_KEYS;
    QStringList skipSerializeAttributes() const;

    QString label() const;

private:
    QMetaObject::Connection m_updateSongLabelConnection;

protected:
    void serialize(QDataStream &out) const;
    void deserialize(QDataStream &in);

public:
    Ratio similarity(const DatabaseItemBase *other) const;
};

Q_DECLARE_METATYPE(SetlistItem*)
Q_DECLARE_METATYPE(SetlistItem::Type)

DECLARE_ENUM_STREAM_OPERATORS(SetlistItem::Type)

#endif // SETLISTITEM_H
