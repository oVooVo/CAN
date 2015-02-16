#include "chord.h"
#include "global.h"


// each chord ends implicitly with a tick (`). ticks are never displayed.

const QString CHORD_EXTENSION_PATTERN = "(maj|min|5|7th|maj7|min7|sus4|sus2|°|dim|dim7|aug|6|min6|"\
                                        "9|min9|maj9|11|min11|maj11|13|min13|maj13|add9|maj7th|7)*(\\(\\w+\\))?$" ;

Chord::Chord(const QString token, int transpose, int m_column) :
    m_isValid( parse(token, transpose) ),
    m_column( m_column )
{
}

QString Chord::flat(const QString& s)
{
    return QString("%1%2").arg(s).arg(QChar(0x266D));
}

QString Chord::sharp(const QString& s)
{
    return QString("%1%2").arg(s).arg(QChar(0x266F));
}

QString Chord::baseString( EnharmonicPolicy epolicy, int tranpose ) const
{
    switch ((m_base + tranpose) % 12)
    {
    case 0:
        return "C";
    case 1:
        switch (epolicy)
        {
        case Sharp:
        case Natural:
            return sharp("C");
        case Flat:
            return flat("D");
        }
    case 2:
        return "D";
    case 3:
        switch (epolicy)
        {
        case Sharp:
            return sharp("D");
        case Natural:
        case Flat:
            return flat("E");
        }
    case 4:
        return "E";
    case 5:
        return "F";
    case 6:
        switch (epolicy)
        {
        case Sharp:
        case Natural:
            return sharp("F");
        case Flat:
            return flat("G");
        }
    case 7:
        return "G";
    case 8:
        switch (epolicy)
        {
        case Sharp:
            return sharp("G");
        case Flat:
        case Natural:
            return flat("A");
        }
    case 9:
        return "A";
    case 10:
        switch (epolicy)
        {
        case Sharp:
            return sharp("A");
        case Flat:
        case Natural:
            return flat("B");
        }
    case 11:
        return "B";
    default:
        return "?";
    }
}

QString Chord::toString(int tranpose, MinorPolicy mpolicy, EnharmonicPolicy epolicy) const
{
    QString text = baseString(epolicy, tranpose);

    if (isMinor())
    {
        switch (mpolicy)
        {
        case LowerCase:
            text = text.toLower();
            break;
        case FollowingM:
            text += "m";
            break;
        }
    }

    text += attachment();

    return text;
}

int parseBase( const QChar & c )
{
    switch (c.toUpper().cell())
    {
    case 'C':
        return 0;
    case 'D':
        return 2;
    case 'E':
        return 4;
    case 'F':
        return 5;
    case 'G':
        return 7;
    case 'A':
        return 9;
    case 'B':
        return 11;
    default:
        return -1;
    }
}

bool ifStartsWithTake( QString & string, const QString & pattern )
{
    if (string.startsWith(pattern))
    {
        string = string.mid( pattern.length() );
        return true;
    }
    return false;
}

bool Chord::parse(QString text, int transpose)
{
    if (text.isEmpty())
    {
        return false;
    }

    QChar baseChar = text[0];
    m_base = parseBase(baseChar);
    if (m_base < 0)
    {
        return false;
    }

    text = text.mid(1);

    if ( ifStartsWithTake( text, "es" )
     || (!text.startsWith("sus") && ifStartsWithTake( text, "s") )
     ||  ifStartsWithTake( text, "b" )
     ||  ifStartsWithTake( text, flat())    )
    {
        m_base--;
    }

    if ( ifStartsWithTake( text, "is" )
     ||  ifStartsWithTake( text, "#" )
     ||  ifStartsWithTake( text, sharp() ) )
    {
        m_base++;
    }

    m_base -= transpose;
    m_base %= 12;
    m_base += 12;
    m_base %= 12;

    if ( baseChar.isLower() || ifStartsWithTake( text, "m" ) )
    {
        m_isMinor = true;
    }
    else
    {
        m_isMinor = false;
    }

    m_attachment = text;
    if (QRegExp( CHORD_EXTENSION_PATTERN ).exactMatch( m_attachment ))
    {
        return true;
    }
    else
    {
        return false;
    }
}




























