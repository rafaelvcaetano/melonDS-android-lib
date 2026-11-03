#ifndef DSPHLEBOUNDS_H
#define DSPHLEBOUNDS_H

#include "types.h"

namespace melonDS
{
struct DSPHLEBounds
{
    static constexpr u32 PageWords = 0x4000;
    static constexpr u32 MaxG711Samples = 0x100000;

    static constexpr bool PipeValid(u32 base, u32 byteLength, u32 readPointer, u32 writePointer)
    {
        const u32 baseOffset = base & (PageWords - 1);
        return byteLength > 0 && (byteLength & 1) == 0 &&
            byteLength / 2 <= PageWords - baseOffset &&
            (readPointer & 1) == 0 && (writePointer & 1) == 0 &&
            readPointer < byteLength && writePointer < byteLength;
    }

    static constexpr bool MemoryTransferValid(u32 byteLength)
    {
        return (byteLength & 1) == 0;
    }

    static constexpr bool MicBufferValid(u32 dataAddress, u32 length, u32 writePosition)
    {
        const u32 pageOffset = dataAddress & (PageWords - 1);
        return length > 0 && length <= PageWords - pageOffset && writePosition < length;
    }

    static constexpr u32 ScaledDimension(u32 source, u32 scale)
    {
        return static_cast<u32>((static_cast<u64>(source) * scale) / 1000);
    }

    static constexpr bool YuvTransferValid(u32 byteLength)
    {
        return (byteLength & 3) == 0 && byteLength <= (UINT32_MAX - 200) / 12;
    }

    static constexpr bool G711LengthValid(u32 samples)
    {
        return samples <= MaxG711Samples;
    }

    static constexpr bool G711TimingValid(u32 samples, u32 cyclesPerSample)
    {
        return static_cast<u64>(samples) * cyclesPerSample + 200 <= UINT32_MAX;
    }

    static constexpr bool RegionSpanFits(u32 offset, u32 byteLength, u32 mask)
    {
        return offset <= mask && byteLength <= static_cast<u64>(mask) + 1 - offset;
    }

    static constexpr bool NWRAMSpanFits(u32 start, u32 end, u32 address, u32 byteLength)
    {
        constexpr u32 NWRAMStart = 0x03000000;
        constexpr u32 NWRAMEnd = 0x04000000;
        const u32 effectiveStart = start < NWRAMStart ? NWRAMStart : start;
        const u32 effectiveEnd = end > NWRAMEnd ? NWRAMEnd : end;
        return effectiveStart < effectiveEnd && address >= effectiveStart && address < effectiveEnd &&
            byteLength <= static_cast<u64>(effectiveEnd) - address;
    }

    static constexpr bool ScalingValid(
        u32 filter, u32 sourceWidth, u32 sourceHeight, u32 destinationWidth, u32 destinationHeight)
    {
        if (sourceWidth == 0 || sourceHeight == 0 || destinationWidth == 0 || destinationHeight == 0)
            return false;
        if (filter == 10)
            return sourceWidth >= 3 && sourceHeight >= 3 &&
                sourceWidth % 3 == 0 && sourceHeight % 3 == 0 && sourceWidth <= PageWords / 3;
        if (destinationWidth < 2 || destinationHeight < 2)
            return false;
        if (filter == 3)
            return sourceWidth >= 4 && sourceHeight >= 4 &&
                sourceWidth <= PageWords / 4 && destinationWidth <= PageWords;
        if (filter == 2)
            return sourceWidth >= 2 && sourceHeight >= 2 &&
                sourceWidth <= PageWords / 2 && destinationWidth <= PageWords;
        return sourceWidth >= 2 && sourceHeight >= 2 &&
            sourceWidth <= PageWords && destinationWidth <= PageWords;
    }
};
}

#endif // DSPHLEBOUNDS_H
