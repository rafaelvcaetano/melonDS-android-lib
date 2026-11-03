#ifndef GBAFLASHBOUNDS_H
#define GBAFLASHBOUNDS_H

#include <limits>
#include "types.h"

namespace melonDS
{
struct GBAFlashBounds
{
    static constexpr u32 BankSize = 0x10000;
    static constexpr u32 SectorSize = 0x1000;
    static constexpr u32 MaxFlashSize = 2 * BankSize;
    static constexpr u32 InvalidOffset = std::numeric_limits<u32>::max();

    static constexpr u32 UsableLength(u32 saveLength)
    {
        return saveLength < MaxFlashSize ? saveLength : MaxFlashSize;
    }

    static constexpr bool BankFits(u32 bank, u32 saveLength)
    {
        return bank < 2 && bank * BankSize <= UsableLength(saveLength) &&
            BankSize <= UsableLength(saveLength) - bank * BankSize;
    }

    static constexpr u32 ByteOffset(u32 bank, u32 address, u32 saveLength)
    {
        if (!BankFits(bank, saveLength)) return InvalidOffset;
        const u32 offset = bank * BankSize + (address & 0xFFFF);
        return offset < UsableLength(saveLength) ? offset : InvalidOffset;
    }

    static constexpr u32 SectorOffset(u32 bank, u32 address, u32 saveLength)
    {
        const u32 byteOffset = ByteOffset(bank, address & ~(SectorSize - 1), saveLength);
        if (byteOffset == InvalidOffset || SectorSize > UsableLength(saveLength) - byteOffset)
            return InvalidOffset;
        return byteOffset;
    }
};
}

#endif // GBAFLASHBOUNDS_H
