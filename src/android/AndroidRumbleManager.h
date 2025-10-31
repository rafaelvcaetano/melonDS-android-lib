#ifndef ANDROIDRUMBLEMANAGER_H
#define ANDROIDRUMBLEMANAGER_H

#include "types.h"

using namespace melonDS;

namespace MelonDSAndroid
{
    class AndroidRumbleManager {
    public:
        virtual void startRumble(u32 duration) = 0;
        virtual void stopRumble() = 0;
        virtual ~AndroidRumbleManager() {};
    };
}

#endif //ANDROIDRUMBLEMANAGER_H
