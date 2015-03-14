#include "mainwindow.h"
#include "global.h"
#include "application.h"

#define FILENAME "/media/Volume/Musik/Coldplay/X&Y/Fix You.mp3"

int main(int argc, char *argv[])
{
    Application a(argc, argv);

    a.fileIndex().restore();

////     load stylesheet
//    QFile styleSheetFile(":/style/style.css");
//    assert( styleSheetFile.open(QIODevice::ReadOnly) );
//    a.setStyleSheet( styleSheetFile.readAll() );

    MainWindow m;
    m.show();

    return a.exec();
}
