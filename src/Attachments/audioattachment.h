#ifndef AUDIOATTACHMENT_H
#define AUDIOATTACHMENT_H

#include "Attachments/indexedfileattachment.h"

class AudioAttachment : public IndexedFileAttachment
{
    Q_OBJECT
    DECL_CREATABLE( AudioAttachment );
public:
    AudioAttachment();
    void copy(Attachment *&attachment) const;
    void loadContent();

    QStringList acceptedEndings() const { return QStringList() << "mp3" << "ogg"; }
};

#endif // AUDIOATTACHMENT_H
