#ifndef DSISAVEGEOMETRY_H
#define DSISAVEGEOMETRY_H

#include "types.h"

namespace melonDS::DSi_NAND
{
struct SaveGeometry
{
    u16 sectorsPerTrack = 0;
    u16 headCount = 0;
    u16 totalSectors = 0;
    u16 rootEntries = 0;
    u8 sectorsPerCluster = 0;
    u16 fatSectors = 0;
};

inline bool CalculateSaveGeometry(u32 length, SaveGeometry& result)
{
    constexpr u32 SectorSize = 0x200;
    if (length < SectorSize || length % SectorSize != 0)
        return false;

    const u32 maxSectors = length / SectorSize;
    if (maxSectors == 0 || maxSectors > 0xFFFF)
        return false;

    u32 trackSize = 1;
    u32 headCount = 1;
    u32 totalSectors = 1;
    while (true)
    {
        const u64 nextHeads = static_cast<u64>(trackSize) * (headCount + 1) * (headCount + 1);
        if (nextHeads > maxSectors)
            break;

        ++headCount;
        totalSectors = static_cast<u32>(nextHeads);

        ++trackSize;
        const u64 nextTrack = static_cast<u64>(trackSize) * headCount * headCount;
        if (nextTrack <= maxSectors)
            totalSectors = static_cast<u32>(nextTrack);
    }

    const u64 finalTrack = static_cast<u64>(trackSize + 1) * headCount * headCount;
    if (finalTrack <= maxSectors)
    {
        ++trackSize;
        totalSectors = static_cast<u32>(finalTrack);
    }

    const u32 sectorsPerCluster = totalSectors > (8U << 10) ? 8 :
        (totalSectors > (1U << 10) ? 4 : 1);
    const u32 alignedSectors = (totalSectors + sectorsPerCluster - 1) / sectorsPerCluster;
    const u32 fatBytes = ((alignedSectors + 1) / 2) * 3;
    const u32 fatSectors = (fatBytes + SectorSize - 1) / SectorSize;
    if (trackSize > 0xFFFF || headCount > 0xFFFF || fatSectors > 0xFFFF)
        return false;

    result.sectorsPerTrack = static_cast<u16>(trackSize);
    result.headCount = static_cast<u16>(headCount);
    result.totalSectors = static_cast<u16>(totalSectors);
    result.rootEntries = length < 0x8C000 ? 0x20 : 0x200;
    result.sectorsPerCluster = static_cast<u8>(sectorsPerCluster);
    result.fatSectors = static_cast<u16>(fatSectors);
    return true;
}
}

#endif // DSISAVEGEOMETRY_H
