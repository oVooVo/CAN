#ifndef SONG_H
#define SONG_H

#include "taggable.h"
#include <QList>
#include "Attachments/attachment.h"
#include <QVariantMap>
#include <QDateTime>
#include "program.h"

class SongDatabase;
class Song : public QObject, public Taggable
{
    Q_OBJECT
public:
    Song(SongDatabase *database);
    ~Song();
private:
    SongDatabase* m_songDatabase;



public:
    bool restoreFromJsonObject(const QJsonObject &json);
    QJsonObject toJsonObject() const;
    SongDatabase* database() const { return m_songDatabase; }
    Song* copy() const;


    /////////////////////////////////////////////////
    ////
    ///  Attributes
    //
    /////////////////////////////////////////////////
private:
    QVariantList m_attributes;      // Title, Artist, CreationDateTime, [User data ...]
public:
    /**
     * @brief attribute return attribute at index or QVariant if index does not exist.
     * @param index
     * @return
     */
    QVariant attribute(int index) const;

    /**
     * @brief attribute return reference to attribute at index.
     *  creates the attribute if it does not exist.
     * @param index
     * @return
     */
    QVariant& attribute(int index);
    QVariantList attributes() const { return m_attributes; }
    QMap<QString, QString> stringAttributes() const;

    void setAttribute(int index, const QVariant &data);
    void insertAttribute(int index, const QVariant &data);
    void removeAttribute(int index);
    QString title() const;
    QString artist() const;
    QTime duration() const;
    QString description() const { return title(); }
    QDateTime creationTime() const;

signals:
    void attributesEdited();



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

    /////////////////////////////////////////////////
    ////
    ///  Programs
    //
    /////////////////////////////////////////////////
    //TODO currently, only one progam per song is supported. change that!
public:
    Program program() const { return m_program; }
    void setProgram( const Program& program ) { m_program = program; }
private:
    Program m_program = Program();



};

#endif // SONG_H
