#ifndef CARTRIDGEIMAGEBOUNDS_H
#define CARTRIDGEIMAGEBOUNDS_H

#include "NDS_Header.h"

namespace melonDS
{
struct CartridgeImageBounds
{
    static constexpr u64 MaxNDSLength = 512ULL * 1024 * 1024;
    static constexpr u64 MaxGBALength = 32ULL * 1024 * 1024;

    static constexpr bool IsNDSLengthSupported(u64 length)
    {
        return length >= sizeof(NDSHeader) && length <= MaxNDSLength;
    }

    static constexpr bool IsGBALengthSupported(u64 length)
    {
        return length >= 0xC0 && length <= MaxGBALength;
    }

    static constexpr bool RangeFits(u32 offset, u32 size, u64 length)
    {
        return offset <= length && size <= length - offset;
    }

    static constexpr u32 DirectBootReadWidth(u64 length, u64 offset)
    {
        if (offset >= length) return 0;
        const u64 remaining = length - offset;
        return remaining < sizeof(u32) ? static_cast<u32>(remaining) : sizeof(u32);
    }

    static bool ValidateNDS(const NDSHeader& header, u64 length)
    {
        if (!IsNDSLengthSupported(length)) return false;
        if (!RangeFits(header.ARM9ROMOffset, header.ARM9Size, length) ||
            !RangeFits(header.ARM7ROMOffset, header.ARM7Size, length) ||
            !RangeFits(header.FNTOffset, header.FNTSize, length) ||
            !RangeFits(header.FATOffset, header.FATSize, length) ||
            !RangeFits(header.ARM9OverlayOffset, header.ARM9OverlaySize, length) ||
            !RangeFits(header.ARM7OverlayOffset, header.ARM7OverlaySize, length) ||
            !RangeFits(header.DebugROMOffset, header.DebugSize, length))
            return false;

        if (header.ARM9ROMOffset >= 0x4000 && header.ARM9ROMOffset < 0x8000 &&
            !RangeFits(header.ARM9ROMOffset, 0x800, length))
            return false;

        if (header.BannerOffset)
        {
            const u32 bannerSize = header.IsDSi() ? 0x23C0 : 0xA40;
            if (!RangeFits(header.BannerOffset, bannerSize, length)) return false;
        }

        if (header.IsDSi())
        {
            if (!RangeFits(header.DSiARM9iROMOffset, header.DSiARM9iSize, length) ||
                !RangeFits(header.DSiARM7iROMOffset, header.DSiARM7iSize, length) ||
                !RangeFits(header.DSiModcrypt1Offset, header.DSiModcrypt1Size, length) ||
                !RangeFits(header.DSiModcrypt2Offset, header.DSiModcrypt2Size, length))
                return false;
        }
        return true;
    }
};
}

#endif // CARTRIDGEIMAGEBOUNDS_H
