#ifndef MPPACKETVALIDATION_H
#define MPPACKETVALIDATION_H

#include <cstddef>
#include "MPInterface.h"

namespace melonDS
{
struct MPPacketValidation
{
    static constexpr bool PayloadFits(size_t packetLength, u32 claimedLength, u32 maximum)
    {
        return packetLength >= sizeof(MPPacketHeader) && claimedLength <= maximum &&
            claimedLength <= packetLength - sizeof(MPPacketHeader);
    }

    static constexpr bool HeaderValid(const MPPacketHeader& header, size_t packetLength)
    {
        const u32 kind = header.Type & 0xFFFFU;
        const u32 aid = header.Type >> 16;
        if (header.Magic != 0x4946494E || header.SenderID >= 16 || kind > 3) return false;
        if (kind == 2)
            return aid >= 1 && aid <= 15 && PayloadFits(packetLength, header.Length, 1024);
        return aid == 0 && PayloadFits(packetLength, header.Length, 2048);
    }
};
}

#endif
