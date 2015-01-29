#include "fileindex.h"
#include "global.h"
#include <QDir>
#include "file.h"
#include "global.h"


FileIndex::FileIndex()
{
    addEntry("/");
    addEntry("/home/");
    addEntry("/home/pascal/test/abc/123/b/");
}

int FileIndex::rowCount(const QModelIndex &parent) const
{
    int c = -1;
    if (parent.isValid())
    {
        c = entry(parent)->children().length();
    }
    else
    {
        c = m_entries.length();
    }

    return c;
}

int FileIndex::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;   // filename, status, recursive stat, file/dir
}

QModelIndex FileIndex::parent(const QModelIndex &child) const
{
    FileIndexEntry* e = entry(child);
    if (e->parent())
    {
        int row = e->parent()->children().indexOf(e);
        return createIndex(row, 0, e->parent());
    }
    else
    {
        return QModelIndex();
    }
}

FileIndexEntry* FileIndex::entry(const QModelIndex &index) const
{
    return static_cast<FileIndexEntry*>(index.internalPointer());
}


QVariant FileIndex::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
            return entry(index)->path();
        case 1:
            return entry(index)->absolutePath();
        case 2:
            return entry(index)->isDir() ? "[DIR]" : "[FILE]";
        default:
            return QVariant();
        }
    }
    else
    {
        return QVariant();
    }
}

QModelIndex FileIndex::index(int row, int column, const QModelIndex &parent) const
{
    FileIndexEntry* current = 0;
    if (parent.isValid())
    {
        FileIndexEntry* parentEntry = entry(parent);
        current = parentEntry->children()[row];
    }
    else
    {
        current = m_entries[row];
    }
    return createIndex(row, column, current);
}


FileIndexEntry* FileIndex::find(QString &path) const
{
    for (FileIndexEntry* e : m_entries)
    {
        FileIndexEntry* match = e->find(path);
        if (match)
        {
            return match;
        }
    }
    return NULL;
}

bool FileIndex::insertRows(int row, int count, const QModelIndex &parent)
{
    assert(m_inputBuffer.size() == count);
    beginInsertRows(parent, row, row + count - 1);
    if (parent.isValid())
    {
        for (int i = 0; i < count; ++i)
        {
            qDebug() << "insert [1]" << row << i;
            entry(parent)->insertChild(row + i, m_inputBuffer[i]);
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            qDebug() << "insert [2]" << row << i;
            m_entries.insert(row + i, m_inputBuffer[i]);
        }
    }
    endInsertRows();
    m_inputBuffer.clear();
    return true;
}

QModelIndex FileIndex::indexOf(FileIndexEntry *entry) const
{
    if (entry->isRoot())
    {
        int row = m_entries.indexOf(entry);
        return index( row, 0, QModelIndex());
    }
    else
    {
        FileIndexEntry* parent = entry->parent();
        int row = entry->parent()->children().indexOf(entry);
        return index( row, 0, indexOf(parent));
    }
}

void FileIndex::addEntry(const QString & absolutePath)
{
    QString path = absolutePath;
    FileIndexEntry* nearest = find(path);
    if (nearest)
    {
        fillGap( nearest->absolutePath(), absolutePath );
    }
    else
    {
        fillGap( "", absolutePath );
    }
    path = absolutePath;
    nearest = find(path);

    if (absolutePath.isEmpty())
    {
        // nearest is desired entry
        WARNING << "FileIndex already contains " << absolutePath;
    }
    else
    {
        if (nearest)
        {
            assert( nearest->isDir() ); // do not add a file in a file.

            Dir* parentDir = static_cast<Dir*>(nearest);
            m_inputBuffer.append( FileIndexEntry::create( NULL, path, absolutePath ) );
            qDebug() << "insert rows [1] " << path;
            insertRows( parentDir->children().size() , 1, indexOf(parentDir) );
        }
        else
        {
            fillGap( "", absolutePath );
            m_inputBuffer.append( FileIndexEntry::create( NULL, path, absolutePath ) );
            qDebug() << "insert rows [2] " << path;
            insertRows( m_entries.size(), 1, QModelIndex() );
        }
    }
}

void FileIndex::fillGap(const QString &nearest, const QString &current)
{
    QStringList nList = nearest.split("/");
    QStringList cList = current.split("/");

    while (!nList.isEmpty() && !cList.isEmpty() && nList.first() == cList.first())
    {
        cList.removeFirst();
        nList.removeFirst();
    }
    qDebug() << "fill gap: " << nList << cList;
    assert(nList.isEmpty() || (nList.first() == "" && nList.size() == 1));

    QString newEntry = nearest;
    for (int i = 0; i < cList.size() - 2; ++i)
    {
        newEntry += cList[i];
        newEntry += "/";
        addEntry(newEntry);
    }

}

























