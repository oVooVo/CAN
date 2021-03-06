#include "songdatabase.h"

#include <QSize>
#include <QPainter>
#include <QIcon>
#include <QDataStream>
#include <QStringList>

#include "Commands/DatabaseCommands/databasenewitemcommand.h"
#include "Project/project.h"
#include "songdatabasesortproxy.h"
#include "Attachments/attachment.h"
#include "Attachments/AudioAttachment/audioattachment.h"
#include "chord.h"

SongDatabase::SongDatabase(Project *project) :
    Database(project)
{
}

int SongDatabase::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_ASSERT(!parent.isValid());
    return 9;
}

QString peopleNames(const QStringList& names, const QBitArray& peoples)
{
    QStringList ps;
    for (int i = 0; i < peoples.size(); ++i)
    {
        if (peoples[i])
        {
            ps << names[i];
        }
    }
    return ps.join(", ");
}

QIcon buildIcon(QPixmap midiMap, QPixmap songMap)
{
    if (midiMap.isNull())
    {
        return songMap;
    }
    if (songMap.isNull())
    {
        return midiMap;
    }

    QSize singleSize = QSize(qMax(midiMap.width(), songMap.width()), qMax(midiMap.height(), songMap.height()));
    midiMap = midiMap.scaled(singleSize);
    songMap = songMap.scaled(singleSize);

    QPixmap pixmap(QSize(singleSize.width() * 2 + 3, singleSize.height()));
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.drawPixmap(0, 0, midiMap);
    painter.drawPixmap(singleSize.width() + 3, 0, songMap);


    return QIcon(pixmap);
}


AudioAttachment::IndexedFileStatus audioAttachmentStatus(const Song* song)
{
    AudioAttachment::IndexedFileStatus best = AudioAttachment::NoAudioAttachment;
    for (const Attachment* a : song->attachments())
    {
        if (a->inherits("AudioAttachment"))
        {
            best = qMax(best, static_cast<const AudioAttachment*>(a)->status());
        }
    }
    return best;
}

QVariant SongDatabase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            switch (section)
            {
            case 0:
                return tr("Title");
            case 1:
                return tr("Artist");
            case 2:
                return tr("Duration");
            case 3:
                return tr("Key");
            case 4:
                return tr("Labels");
            case 5:
                return tr("State");
            case 6:
                return tr("Solos");
            case 7:
                return tr("Singers");
            case 8:
                return tr("Comments");
            }

        default:
            return QVariant();
        }
    }
    else
    {
        QPixmap midiIcon, songIcon;
        switch (role)
        {
        case Qt::SizeHintRole:
            return QSize(24, 0);
        case Qt::DecorationRole:
#ifdef HAVE_PROGRAM
            if (m_items[section]->program().isValid())
            {
                midiIcon = QPixmap(":/icons/icons/midi.png");
            }
#endif
            switch (audioAttachmentStatus(m_items[section]))
            {
            case IndexedFileAttachment::NoAudioAttachment:
                songIcon = QPixmap();
                break;
            case IndexedFileAttachment::NoFileSet:
                songIcon = QPixmap(":/icons/icons/songNoFile.png");
                break;
            case IndexedFileAttachment::FileNotAvailable:
                songIcon = QPixmap(":/icons/icons/songNotAvailable.png");
                break;
            case IndexedFileAttachment::FileAvailable:
                songIcon = QPixmap(":/icons/icons/song1.png");
                break;
            }
            return buildIcon(midiIcon, songIcon);
        }
        return QVariant();

    }
}

Qt::ItemFlags SongDatabase::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    if (index.column() < 6)
    {
        flags |= Qt::ItemIsEditable;
    }
    else
    {
        // other columns use dialogs for editing
    }
    return flags;
}

QList<int> toIntList(const QVariant& value)
{
    QList<int> ints;
    for (const QVariant& val : value.toList())
    {
        ints << val.toInt();
    }
    return ints;
}

Qt::DropActions SongDatabase::supportedDragActions() const
{
    return Qt::LinkAction;
}

QString SongDatabase::itemName(int n) const
{
    return tr("Song(s)", "", n);
}

QStringList SongDatabase::peoples() const
{
    QStringList peoples;
    for (const Song* song : items())
    {
        peoples << song->attribute("singers").toStringList();
        peoples << song->attribute("soloPlayers").toStringList();
    }
    peoples.removeDuplicates();
    return peoples;
}

void SongDatabase::serialize(QDataStream& out) const
{
    out << m_peoples;
    Database::serialize(out);
}

void SongDatabase::deserialize(QDataStream& in)
{
    in >> m_peoples;
    Database::deserialize(in);
}
