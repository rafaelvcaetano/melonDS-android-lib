#include "InputAndroid.h"
#include "../NDS.h"

namespace MelonDSAndroid
{
    void touchScreen(u16 x, u16 y)
    {
        NDS::TouchScreen(x, y);
    }

    void releaseScreen()
    {
        NDS::ReleaseScreen();
    }

    void pressKey(u32 key)
    {
        // Special handling for Lid input
        if (key == 16 + 7)
            NDS::SetLidClosed(true);
        else
            NDS::PressKey(key);
    }

    void releaseKey(u32 key)
    {
        // Special handling for Lid input
        if (key == 16 + 7)
            NDS::SetLidClosed(false);
        else
            NDS::ReleaseKey(key);
    }
}

