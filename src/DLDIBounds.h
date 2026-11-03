#ifndef DLDIBOUNDS_H
#define DLDIBOUNDS_H

#include "types.h"

namespace melonDS
{
struct DLDIBounds
{
    static constexpr bool RangeFits(u32 offset, u32 length, u32 capacity)
    {
        return offset <= capacity && length <= capacity - offset;
    }

    static constexpr bool SearchProbeFits(u32 offset, u32 sectionLength)
    {
        return RangeFits(offset, 12, sectionLength);
    }

    static constexpr bool PatchFits(
        u32 offset, u32 reservedCapacity, u32 initializedLength,
        u32 declaredSize, u32 binaryLength)
    {
        return reservedCapacity >= 0x80 && initializedLength >= 0x80 &&
            initializedLength <= declaredSize && declaredSize <= reservedCapacity &&
            RangeFits(offset, reservedCapacity, binaryLength);
    }

    static constexpr bool FixRangeFits(u32 start, u32 end, u32 patchLength)
    {
        return start <= end && end <= patchLength &&
            (start & 3U) == 0 && (end & 3U) == 0;
    }
};
}

#endif // DLDIBOUNDS_H
