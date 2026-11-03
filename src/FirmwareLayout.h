#ifndef FIRMWARELAYOUT_H
#define FIRMWARELAYOUT_H

#include "types.h"

namespace melonDS
{
struct FirmwareLayout
{
    static constexpr bool IsSupportedLength(u64 length)
    {
        return length == 0x20000 || length == 0x40000 || length == 0x80000;
    }

    static constexpr bool Validate(u32 length, u32 userDataOffset, bool dsi, u32 wifiConfigLength)
    {
        if (!IsSupportedLength(length) || length < 0x2C ||
            wifiConfigLength > length - 0x2C)
            return false;
        if (userDataOffset < 0x400 || userDataOffset > length ||
            2 * 0x100 > length - userDataOffset)
            return false;
        const u32 accessPointOffset = userDataOffset - 0x400;
        if (3 * 0x100 > length - accessPointOffset)
            return false;
        if (dsi)
        {
            if (userDataOffset < 0xA00)
                return false;
            const u32 extendedOffset = userDataOffset - 0xA00;
            if (3 * 0x200 > length - extendedOffset)
                return false;
        }
        return true;
    }
};
}

#endif // FIRMWARELAYOUT_H
