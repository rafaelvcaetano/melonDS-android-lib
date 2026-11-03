#ifndef DSI_NWIFIBOUNDS_H
#define DSI_NWIFIBOUNDS_H

#include "types.h"

namespace melonDS
{
struct DSi_NWifiBounds
{
    static constexpr bool MailboxPayloadValid(u32 declaredLength, u32 availableLength)
    {
        return declaredLength <= 0x600 && declaredLength <= availableLength;
    }

    static bool ReadLE32(const u8* data, u32 size, u32 offset, u32& value)
    {
        if (!data || offset > size || size - offset < sizeof(u32))
            return false;

        value = static_cast<u32>(data[offset]) |
            (static_cast<u32>(data[offset + 1]) << 8) |
            (static_cast<u32>(data[offset + 2]) << 16) |
            (static_cast<u32>(data[offset + 3]) << 24);
        return true;
    }
};
}

#endif // DSI_NWIFIBOUNDS_H
