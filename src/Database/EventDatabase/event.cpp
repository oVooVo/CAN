#include "event.h"
#include "setlist.h"
#include "application.h"
#include "setlistitem.h"

Event::Event( Database<Event>* database, const QDateTime& beginning, Type type, const QString & label) :
    DatabaseItem(database),
    m_setlist( new Setlist(this) )
{
    addAttributeKey("type");
    addAttributeKey("beginning");
    addAttributeKey("label");
    addAttributeKey("notices");

    setAttribute("beginning", beginning);
    setAttribute("type", type);
    setAttribute("label", label);
}

Event::~Event()
{
    delete m_setlist;
    m_setlist = nullptr;
}

QString Event::description() const
{
    if (attributeDisplay("label").isEmpty())
    {
        return attributeDisplay("type");
    }
    else
    {
        return attributeDisplay("label");
    }
}

QString Event::eventTypeName(Type type)
{
    return eventTypeNames()[static_cast<int>(type)];
}

QString Event::label() const
{
    QString label = attributeDisplay("label");
    if (!label.isEmpty())
    {
        label = attributeDisplay("type");
    }
    return QString("%1 (%2)").arg(label, attributeDisplay("beginning"));
}

QStringList Event::eventTypeNames()
{
    return QStringList({ app().translate("Event", "Rehearsal"), app().translate("Event", "Gig"), app().translate("Event", "Other") });
}

QString Event::attributeDisplay(const QString &key) const
{
    QVariant attribute = DatabaseItem::attribute(key);
    if (key == "beginning" || key == "creationDateTime")
    {
        return attribute.toDateTime().toString(app().dateTimeFormat());
    }
    if (key == "type")
    {
        return eventTypeName(attribute.value<Type>());
    }
    if (attribute.isNull())
    {
        return "";
    }
    if (attribute.canConvert<QString>())
    {
        return attribute.toString();
    }
    Q_UNREACHABLE();
    return "";
}

bool Event::needsSong(const Song *song) const
{
    for (SetlistItem* item : setlist()->items())
    {
        if (item->attribute("song").value<const Song*>() == song)
        {
            return true;
        }
    }
    return false;
}

void Event::setSetlist(const Setlist* newSetlist)
{
    setlist()->setSetlistItems(newSetlist->items());
}

void Event::serialize(QDataStream& out) const
{
    DatabaseItem::serialize(out);
    out << m_setlist;
}

void Event::deserialize(QDataStream& in)
{
    DatabaseItem::deserialize(in);
    in >> m_setlist;
}

Ratio Event::similarity(const DatabaseItemBase *other) const
{
    SIMILARITY_BEGIN_CHECK(Event);

    Ratio r = DatabaseItem::similarity(other);

    QMultiMap<double, QPair<SetlistItem*, SetlistItem*>> map = Ratio::sortSimilar<SetlistItem>(setlist()->items(), otherEvent->setlist()->items());
    r += Ratio::similarMapToRatio<SetlistItem>(map, ::preference<double>("SetlistSimilarityThreshold"));

    return r;
}


DEFINE_ENUM_STREAM_OPERATORS(Event::Type)






