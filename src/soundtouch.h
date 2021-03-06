#ifndef SOUNDTOUCH_H
#define SOUNDTOUCH_H

#include <Qt>

#if 0
    #ifdef Q_OS_ANDROID
        //TODO
    #elif defined Q_OS_LINUX
        #include <soundtouch/SoundTouch.h>
        #define HAVE_SOUNDTOUCH
        extern "C"
        {
            #include <libavcodec/avcodec.h>
            #include <libavformat/avformat.h>
            #include <libavutil/avutil.h>
        }
    #elif defined Q_OS_WIN32
        #include "../../soundtouch/include/SoundTouch.h"
        #define HAVE_SOUNDTOUCH
        extern "C"
        {
            #include "libavcodec/avcodec.h"
            #include "libavformat/avformat.h"
            #include "libavutil/avutil.h"
        }
    #else
        #error unsupported OS
    #endif
#endif

#endif // SOUNDTOUCH_H
