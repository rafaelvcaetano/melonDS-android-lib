#ifndef MELONDS_ANDROID_ANDROIDFILEHANDLER_H
#define MELONDS_ANDROID_ANDROIDFILEHANDLER_H

#include <stdio.h>
#include <Platform.h>

namespace MelonDSAndroid
{
    class AndroidFileHandler {
    public:
        virtual FILE* open(const char* path, melonDS::Platform::FileMode mode) = 0;
        virtual ~AndroidFileHandler() {};
    };
}

#endif //MELONDS_ANDROID_ANDROIDFILEHANDLER_H
