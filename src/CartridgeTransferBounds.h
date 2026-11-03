#ifndef CARTRIDGETRANSFERBOUNDS_H
#define CARTRIDGETRANSFERBOUNDS_H

#include "types.h"

namespace melonDS
{
struct CartridgeTransferBounds
{
    static constexpr bool Validate(u32 position, u32 length, u32 direction, u32 capacity)
    {
        return direction <= 1 && length <= capacity && position <= length &&
            (position & 3) == 0 && (length & 3) == 0;
    }

    static constexpr bool WordFits(u32 position, u32 capacity)
    {
        return position <= capacity && sizeof(u32) <= capacity - position;
    }

    static constexpr bool B7LengthValid(u32 length, u32 capacity)
    {
        return length > 0 && length <= capacity;
    }

    static constexpr bool SpanFits(u32 capacity, u32 offset, u32 length)
    {
        return offset <= capacity && length <= capacity - offset;
    }

    static constexpr bool RoundedHalfwordSpanFits(u32 capacity, u32 offset, u32 length)
    {
        const u32 rounded = length + (length & 1U);
        return rounded >= length && SpanFits(capacity, offset, rounded);
    }

    static constexpr bool RoundedWordSpanFits(u32 capacity, u32 offset, u32 length)
    {
        if (length > UINT32_MAX - 3) return false;
        return SpanFits(capacity, offset, (length + 3U) & ~3U);
    }
};
}

#endif // CARTRIDGETRANSFERBOUNDS_H
