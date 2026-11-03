#ifndef WIFIMEMORY_H
#define WIFIMEMORY_H

#include <cstring>

#include "types.h"

namespace melonDS
{
struct WifiMemory
{
    static u16 Read16(const u8* source)
    {
        u16 value;
        std::memcpy(&value, source, sizeof(value));
        return value;
    }

    static u32 Read32(const u8* source)
    {
        u32 value;
        std::memcpy(&value, source, sizeof(value));
        return value;
    }

    static u64 Read64(const u8* source)
    {
        u64 value;
        std::memcpy(&value, source, sizeof(value));
        return value;
    }

    static void Write16(u8* destination, u16 value)
    {
        std::memcpy(destination, &value, sizeof(value));
    }

    static void Write32(u8* destination, u32 value)
    {
        std::memcpy(destination, &value, sizeof(value));
    }

    static void Write64(u8* destination, u64 value)
    {
        std::memcpy(destination, &value, sizeof(value));
    }
};
}

#endif // WIFIMEMORY_H
