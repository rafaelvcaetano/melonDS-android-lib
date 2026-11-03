#ifndef DSIBOOTBOUNDS_H
#define DSIBOOTBOUNDS_H

#include "types.h"

namespace melonDS::DSiBootBounds
{

inline bool SpanValid(u32 offset, u32 size, u32 destination, u64 nandLength)
{
    constexpr u32 MaxBootSectionSize = 16U * 1024U * 1024U;
    if (size == 0 || size > MaxBootSectionSize || (size & 0xFU) != 0)
        return false;
    if (offset > nandLength || size > nandLength - offset)
        return false;
    return static_cast<u64>(destination) + size <= 0x100000000ULL;
}

}

#endif
