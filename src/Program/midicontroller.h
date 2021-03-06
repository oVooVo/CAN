#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include <QFile>
#include <QFileSystemWatcher>

class Program;
class MidiCommand;
class MidiController
{
private:
    MidiController();
public:
    static MidiController* singleton();

    void send( const MidiCommand & command );
    void send( const Program& program);
    void reinit();
    bool connected();

private:
    static MidiController* m_singleton;
    QFile m_midiDevice;
    void initFileSystemWatcher();
    static const QString MIDI_DEVICE_FILENAME;
    QFileSystemWatcher m_midiDeviceWatcher;
};

#endif // MIDICONTROLLER_H
