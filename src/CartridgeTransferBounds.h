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
};
}

#endif // CARTRIDGETRANSFERBOUNDS_H
