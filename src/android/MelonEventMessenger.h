#ifndef MELONEVENTMESSENGER_H
#define MELONEVENTMESSENGER_H

#include <Platform.h>

namespace MelonDSAndroid
{

class MelonEventMessenger
{
public:
    virtual void onRumbleStart(int durationMs) = 0;
    virtual void onRumbleStop() = 0;
    virtual void onEmulatorStop(melonDS::Platform::StopReason reason) = 0;
};

}

#endif // MELONEVENTMESSENGER_H
