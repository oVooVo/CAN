#include "player.h"
#include "global.h"

Player::Player()
{
}

Player::~Player()
{
    if (m_audioOutput)
    {
        m_audioOutput->stop();
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
}

void Player::stop()
{
    if (m_audioOutput)
    {
        if (m_audioOutput->state() != QAudio::StoppedState)
        {
            //m_offset = 0;
            m_audioOutput->stop();
            m_audioOutput->reset();
            m_buffer.stop();
            seek(0);
            emit stopped();
        }

    }
}

void Player::open( const QString &filename )
{
    stop();
    if (m_audioOutput)
    {
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }

    m_buffer.open( filename );

    m_audioOutput = new QAudioOutput( QAudioDeviceInfo::defaultOutputDevice(), m_buffer.audioFormat() );
    m_audioOutput->setNotifyInterval(11);
    connect(m_audioOutput, SIGNAL(notify()), this, SIGNAL(notify()));
    seek(0);
}

void Player::start()
{
    if (m_audioOutput)
    {
        if (m_audioOutput->state() != QAudio::ActiveState)
        {
            m_audioOutput->start( &m_buffer.buffer() );
            emit started();
        }
    }
}

void Player::pause()
{
    // save the position into offset. Else this information will be lost when stopping m_audioOutput.
    // pause shall keep the position, unlike stop.
    if (m_audioOutput)
    {
        if (m_audioOutput->state() != QAudio::StoppedState)
        {
            m_offset = position();
            m_audioOutput->stop();
            emit paused();
        }

    }
}

void Player::seek()
{
    if (m_audioOutput)
    {
        blockSignals(true);
        bool wasActive = (m_audioOutput->state() == QAudio::ActiveState);
        m_audioOutput->stop();
        m_buffer.decode( m_pitch, m_tempo, m_offset );
        if (wasActive)
        {
            m_audioOutput->start( &m_buffer.buffer() );
        }
        blockSignals(false);
    }
}

void Player::seek(double pitch, double tempo, double second)
{
    if (m_pitch != pitch)
    {
        m_pitch = pitch;
        emit pitchChanged();
    }
    if (m_tempo != tempo)
    {
        m_tempo = tempo;
        emit tempoChanged();
    }
    seek( second );
}

void Player::seek(double second)
{
    if (position() != second || m_buffer.buffer().size() == 0)
    {
        m_offset = second;
        seek();
        emit notify();
    }
}

double Player::duration() const
{
    return m_buffer.duration();
}


void Player::setVolume(double volume)
{
    if (m_audioOutput)
    {
        m_audioOutput->setVolume(volume);
    }
}

bool Player::isPlaying() const
{
    return m_audioOutput->state() == QAudio::ActiveState;
}

double Player::position() const
{
    double elapsed = m_audioOutput->elapsedUSecs() / 1000.0 / 1000.0;
    elapsed += m_offset;
    return elapsed;
}










