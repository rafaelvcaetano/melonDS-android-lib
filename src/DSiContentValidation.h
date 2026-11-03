#ifndef DSICONTENTVALIDATION_H
#define DSICONTENTVALIDATION_H

#include <limits>

#include "CartridgeImageBounds.h"
#include "DSi_TMD.h"

namespace melonDS
{
struct DSiContentValidation
{
    // ImportTitle keeps an immutable copy so validation and NAND publication
    // cannot observe different provider bytes. Bound that copy independently
    // from the broader NDS cartridge limit to avoid a 512 MiB eager allocation.
    static constexpr u64 MaxSnapshotLength = 128ULL * 1024 * 1024;

    static constexpr u64 ContentSize(const DSi_TMD::TitleMetadata& tmd)
    {
        u64 result = 0;
        for (u8 byte : tmd.Contents.ContentSize)
            result = (result << 8) | byte;
        return result;
    }

    static constexpr bool LengthSupported(const DSi_TMD::TitleMetadata& tmd, u64 actualLength)
    {
        return actualLength >= 0x4000 &&
            actualLength <= MaxSnapshotLength &&
            actualLength <= std::numeric_limits<u32>::max() &&
            ContentSize(tmd) == actualLength;
    }

    static bool MetadataMatches(const DSi_TMD::TitleMetadata& tmd, const NDSHeader& header,
                                u64 actualLength, const u8 digest[20])
    {
        if (!digest || !LengthSupported(tmd, actualLength) ||
            !CartridgeImageBounds::ValidateNDS(header, actualLength) ||
            header.DSiTitleIDHigh != tmd.GetCategory() ||
            header.DSiTitleIDLow != tmd.GetID())
            return false;

        if ((header.ROMSize && header.ROMSize > actualLength) ||
            (header.DSiTotalROMSize && header.DSiTotalROMSize > actualLength))
            return false;

        u8 difference = 0;
        for (size_t index = 0; index < sizeof(tmd.Contents.ContentSha1Hash); ++index)
            difference |= tmd.Contents.ContentSha1Hash[index] ^ digest[index];
        return difference == 0;
    }
};
}

#endif // DSICONTENTVALIDATION_H
