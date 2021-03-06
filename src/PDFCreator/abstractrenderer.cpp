#include "abstractrenderer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <qmath.h>

#include "Attachments/ChordPatternAttachment/abstractchordpatternattachment.h"
#include "Attachments/ChordPatternAttachment/chordpatternattachment.h"
#include "Attachments/pdfattachment.h"
#include "global.h"
#include "Database/EventDatabase/event.h"
#include "Database/SongDatabase/songdatabase.h"
#include "util.h"
#include "chord.h"
#include "Database/EventDatabase/setlist.h"
#include "pdfcreatorconfig.h"
#include "application.h"

const QString AbstractRenderer::HYPHEN = QChar(0x2014);
const int AbstractRenderer::ALIGN_SONGS__SEPARATE_PAGES = 5;

AbstractRenderer::AbstractRenderer(QSizeF baseSizeMM, Setlist* setlist, const QString &filename) :
    m_baseSizeMM( baseSizeMM ),
    m_setlist( setlist ),
    m_filename( filename )
{
}

AbstractRenderer::~AbstractRenderer()
{
    qDeleteAll( m_pages );
    m_pages.clear();
}

void AbstractRenderer::newPage(Page::Flags flags, const QString& title, int index )
{
    if (index < 0)
    {
        index = m_pages.length();
    }

    Page* page = new Page( m_baseSizeMM, title, flags );
    page->setAdditionalTopMargin(0);
    m_pages.insert( index, page );
    activatePage( index );
}

void AbstractRenderer::activatePage( int i )
{
    Q_ASSERT( i >= 0 && i < m_pages.length() );
    m_currentIndex = i;
}

int AbstractRenderer::currentIndex() const
{
    return m_currentIndex;
}

Page* AbstractRenderer::currentPage() const
{
    return m_pages[ m_currentIndex ];
}

void AbstractRenderer::notifyCurrentTaskChanged( const QString &message)
{
    emit progress( m_currentStep++ );
    emit currentTaskChanged( message + " ..." );
}

void AbstractRenderer::run()
{
    m_currentStep = 0;

    // draw the songs, mark table of contents position and gather
    // table of contents entries.
    paintSetlist();

    if (isInterruptionRequested())
    {
        return;
    }

    notifyCurrentTaskChanged( tr("Table of contents") );
    if (preference<bool>("enable_TableOfContentsPattern"))
    {
        paintTableOfContents();
    }

    if (isInterruptionRequested())
    {
        return;
    }

    notifyCurrentTaskChanged( tr("Align songs") );
    switch (preference<int>("AlignSongs"))
    {
    case 0: // do not align songs
        break;
    case 1: // oddPages
        alignSongs( 0 );
        break;
    case 2: // evenPages
        alignSongs( 1 );
        break;
    case 3: // duplex
        optimizeForDuplex();
        break;
    case 4: // endless, songs will not be aligned either.
    case 5: // separate songs, will not be aligned.
        break;
    }

    if (isInterruptionRequested())
    {
        return;
    }

    notifyCurrentTaskChanged( tr("Generate page numbers") );
    if (preference<bool>("PageNumbers"))
    {
        decoratePageNumbers();
    }

    if (isInterruptionRequested())
    {
        return;
    }

}


bool AbstractRenderer::isEndlessPage() const
{
    return preference<int>("AlignSongs") == 4;
}


//////////////////////////////////////////////
////
///  pdf paint member
//
/////////////////////////////////////////////

void AbstractRenderer::paintHeadline(Page* page, const QString& label)
{
    QPainter* painter = page->painter();
    painter->save();

    QFont font = painter->font();
    font.setBold( true );
    font.setPointSizeF( 15 );
    font.setFamily( "lucida" );
    painter->setFont( font );

    double fontHeight = painter->fontMetrics().height();
    painter->drawText( QPointF(page->leftMargin(), page->topMargin() + fontHeight), label);
    page->setAdditionalTopMargin(2*fontHeight);
    painter->restore();
}

QString labelSong( const Song* song )
{
    QString pattern = QString("%2 %1 %3")
            .arg( AbstractRenderer::HYPHEN )
            .arg( song->attribute("title").toString() )
            .arg( song->attribute("artist").toString() );

    return pattern;
}

bool AbstractRenderer::paintSong(const Song* song)
{
    newPage( Page::SongStartsHere, labelSong(song) );
    QString headline = labelSong( song );
    m_tableOfContents.append( headline );
    paintHeadline( currentPage(), headline );

    int i = 0;
    for ( Attachment* attachment : song->attachments() )
    {
        if (isInterruptionRequested()) return false;
        notifyCurrentTaskChanged( QString(tr("Draw attachment %1 of song %2 ..."))
                                      .arg(song->attribute("title").toString())
                                      .arg(attachment->name())                  );

        if (attachment->isPaintable())
        {
            if (i++ != 0)
            {
                newPage( Page::NothingSpecial );
            }
            currentPage()->painter()->save();
            attachment->paint(this);
            currentPage()->painter()->restore();
        }
    }
    return true;
}

bool AbstractRenderer::pageBreak( const QStringList & lines, const int currentLine, const double heightLeft, const QPainter* painter )
{
    // return whether we must use another page to fit the content
    if (lines[currentLine].isEmpty())
    {
        // we are currently at a paragraph break
        // if next paragraph fits, return false, true otherwise.
        double paragraphHeight = 0;
        double lineHeight =  painter->fontMetrics().height();
        // sum line heights until the next empty line.

        QString paragraph;
        for (int i = currentLine; i < lines.length(); i++ )
        {
            if (i != currentLine && lines[i].isEmpty())
            {
                // paragraph ends
                break;
            }
            QStringList unusedA, unusedB;
            bool isChordLine = Chord::parseLine( lines[i] , unusedA, unusedB );
            if (i != 0)
            {
                if (isChordLine)
                {
                    paragraphHeight += lineHeight * preference<double>("ChordLineSpacing");
                }
                else
                {
                    paragraphHeight += lineHeight * preference<double>("LineSpacing");
                }
            }
            paragraph += lines[i] + "\n";
        }

        return paragraphHeight > heightLeft;
    }
    else
    {
        // we are not at a paragraph break. break if the current line will not fit anymore.
        return painter->fontMetrics().height() > heightLeft;
    }
}

void AbstractRenderer::drawContinueOnNextPageMark(const Page* page, QPainter *painter)
{
    painter->save();
    QFont font = painter->font();
    font.setBold( true );
    font.setPointSizeF( font.pointSizeF() * 3 );
    painter->setFont( font );
    QTextOption option;
    option.setAlignment( Qt::AlignBottom | Qt::AlignRight );
    painter->drawText( page->contentRect(), QChar(0x293E), option );
    painter->restore();
}

void AbstractRenderer::paintSetlist()
{
    m_currentIndex = -1;

    newPage( Page::NothingSpecial );
    QString title = labelSetlist( m_setlist );
    currentPage()->painter()->drawText( currentPage()->contentRect(), Qt::AlignCenter, title );

    m_tableOfContentsPage = currentIndex() + 1;

    int i = 0;
    for ( const Song* song : m_setlist->songs() )
    {
        if (!paintSong( song ))
        {
            // interruption has been requested
            return;
        }
        emit progress(i++);
    }
}

QString labelTableOfContents()
{
    return QObject::tr("Setlist");
}

static void configurePainter(QPainter* painter)
{
    QFont font = painter->font();
    font.setBold( true );
    font.setFamily( "lucida" );
    painter->setFont( font );
}

#define GET_PAINTER painter = currentPage()->painter(); configurePainter(painter); painter->save();
#define RESTORE_PAINTER painter->restore();

void AbstractRenderer::paintTableOfContents()
{
    typedef struct PageNumberStub {
        PageNumberStub( int page, int y ) :
            page( page ),
            y( y )
        {}
        int page;
        int y;
    } PageNumberStub;


    newPage( Page::TableOfContentsStartsHere, "", m_tableOfContentsPage );

    paintHeadline( currentPage(), labelTableOfContents() );

    // draw entries and find places to fill in page numbers
    QList<PageNumberStub> pageNumberStubs;

    QPainter* GET_PAINTER;
    double y = currentPage()->topMargin();
    const double lineHeight = painter->fontMetrics().height();
    while (!m_tableOfContents.isEmpty())
    {
        double spaceLeft = currentPage()->rect().height() - currentPage()->bottomMargin() - lineHeight - y;
        if ( spaceLeft > 0) // content fits on page
        {
            QString currentEntry = m_tableOfContents.takeFirst();
            painter->drawText( QPointF( currentPage()->leftMargin(), y), currentEntry  );
            pageNumberStubs << PageNumberStub( currentIndex(), y );

            y += lineHeight * 1.1;
        }
        else        // content does not fit on page
        {
            if (isEndlessPage())
            {
                currentPage()->growDownMM( -currentPage()->painterUnitsInMM( spaceLeft ) );
            }
            else
            {
                m_tableOfContentsPage++;
                RESTORE_PAINTER;
                newPage( Page::NothingSpecial, "", m_tableOfContentsPage );
                GET_PAINTER;
                y = currentPage()->topMargin();
                // content will fit on page next iteration
            }
        }
    }


    // replace page number stubs

    int pageNumber = -1;
    for (const PageNumberStub& stub : pageNumberStubs)
    {
        // find next pagenumber
        for (int i = pageNumber + 1; i < m_pages.length(); ++i)
        {
            activatePage( i );
            if ( currentPage()->flags() & Page::SongStartsHere)
            {
                pageNumber = i;
                break;
            }
        }

        QString text = QString("%1").arg( pageNumber + 1 );
        RESTORE_PAINTER;
        activatePage( stub.page );
        GET_PAINTER;
        double textWidth = painter->fontMetrics().boundingRect( text ).width();
        double x = currentPage()->contentRect().right() - textWidth;
        painter->drawText( QPointF( x, stub.y), text );

    }
    RESTORE_PAINTER;
}

void AbstractRenderer::alignSongs( int mode )
{
    for (int currentPageNum = 0; currentPageNum < m_pages.length(); ++currentPageNum)
    {
        activatePage( currentPageNum );

        if (    (currentPage()->flags() & Page::SongStartsHere)                    // if song starts here
             && currentPageNum % 2 != mode                                         // if song starts on wrong page
             && lengthOfSong( currentPageNum ) % 2 == 0 )                          // if song length is even. Else, optimizing beginning
                                                                                   // will destroy end-optimum
        {
            newPage( Page::NothingSpecial, "", currentPageNum );
            currentPageNum++;
        }
    }
}

int AbstractRenderer::lengthOfSong( int start )
{
    int savedIndex = currentIndex();

    int length = m_pages.length() - start;
    for (int i = start + 1; i < m_pages.length(); ++i)
    {
        activatePage( i );
        if (currentPage()->flags() & Page::SongStartsHere)
        {
            length = i - start;
            break;
        }
    }

    // activate the page that was active before this function call.
    activatePage( savedIndex );
    return length;
}

void AbstractRenderer::optimizeForDuplex( )
{
    // an odd and the following even page will be printed on the same piece of paper.
    // So ensure that no song is on more than one side of the same paper.
    bool songsStarted = false;
    for (int currentPageNum = 0; currentPageNum < m_pages.length(); ++currentPageNum)
    {
        activatePage( currentPageNum );
        if ( songsStarted
             && !(currentPage()->flags() & Page::SongStartsHere)   // start may be everywhere, but song shall only be continued on odd pages
             && currentPageNum % 2 != 0 )
        {
            newPage( Page::NothingSpecial, "", currentPageNum );
        }
        else if (currentPage()->flags() & Page::SongStartsHere)
        {
            songsStarted = true;
        }
    }
}

void AbstractRenderer::decoratePageNumbers()
{

    // we have to introduce another variable because i must be reset when
    // separate pages-options is on. but i shall not be reset, therefore use j.
    int j = 0;
    for (int i = 0; i < m_pages.length(); ++i)
    {
        activatePage( i );
        QPainter* painter = currentPage()->painter();
        double height = painter->fontMetrics().height();

        if (    (currentPage()->flags() & Page::SongStartsHere       )
             && (preference<int>("AlignSongs") == ALIGN_SONGS__SEPARATE_PAGES ) )
        {
            j = 0;
        }

        double y = currentPage()->rect().height() - currentPage()->bottomMargin();
        painter->drawText( QRectF( 0, y - height/2, currentPage()->rect().width(), height ),
                                   QString("%1").arg( j + 1 ),
                                   QTextOption( Qt::AlignCenter )                       );
        j++;
    }
}


QImage AbstractRenderer::paintChordPatternAttachment(AbstractChordPatternAttachment *attachment)
{
    //TODO
    // save config and replace it at the end.
    app().preferences()->save();

    setPreference("enable_TitlePagePattern", false);
    setPreference("PDFSize", 4); // DinA4
    setPreference("AlignSongs", 4); // we always want to have one page per song.
    setPreference("enable_TableOfContentsPattern", false);
    setPreference("PageNumbers", false);

    AbstractRenderer creator( QPageSize::size( QPageSize::A4, QPageSize::Millimeter ), nullptr, "" );
    creator.m_currentIndex = -1;
    creator.newPage( Page::SongStartsHere );
    attachment->paint(&creator);

    Q_ASSERT(creator.pageCount() == 1);
    creator.activatePage(0);
    Page* page = creator.currentPage();

    double quality = qExp(preference<double>("ImageRenderQuality"));

    QImage canvas(quality * page->sizePainter().toSize(), QImage::Format_ARGB32);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    painter.scale(quality, quality);
    painter.drawPicture(0, 0, page->picture());

    // restore preference
    app().preferences()->restore();

    return canvas;
}

QString AbstractRenderer::labelSetlist(Setlist *setlist)
{
    QString title = QString("Setlist\n\n%1 %2 %1\n\n%3")
            .arg( QChar(0x2014) )
            .arg( setlist->event()->attributeDisplay("label") )
            .arg( setlist->event()->attributeDisplay("beginning") );

    return title;
}


int AbstractRenderer::pageCount() const
{
    return m_pages.length();
}



















