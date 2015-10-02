#include "pdfattachment.h"
#include "application.h"

DEFN_CREATABLE_NAME(PDFAttachment, Attachment, QT_TRANSLATE_NOOP("Creatable", "PDF Attachment"))

PDFAttachment::PDFAttachment() :
    IndexedFileAttachment()
{
    setName( tr("PDF Attachment") );
}

void PDFAttachment::open()
{
#ifdef HAVE_POPPLER
    delete m_document;
    m_document = nullptr;

    if (filename().isEmpty())
    {
        // if filename is empty, dont try to open it.
    }
    else
    {
        m_document = Poppler::Document::load( filename() );
        if (m_document)
        {
            m_document->setRenderHint(Poppler::Document::TextAntialiasing);
        }

    }
#endif
}
