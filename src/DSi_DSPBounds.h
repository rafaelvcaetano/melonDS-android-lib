#ifndef DSI_DSPBOUNDS_H
#define DSI_DSPBOUNDS_H

#include <limits>

#include "types.h"

namespace melonDS
{
struct DSi_DSPBounds
{
    static constexpr u64 MaxCatchUpCycles = 1ULL << 24;

    static constexpr bool RestoredClockValid(u64 dspTimestamp, u64 arm9Timestamp)
    {
        return dspTimestamp <= arm9Timestamp && arm9Timestamp - dspTimestamp <= MaxCatchUpCycles;
    }

    static constexpr u32 CatchUpChunk(u64 backlog)
    {
        return backlog > std::numeric_limits<u32>::max()
            ? std::numeric_limits<u32>::max()
            : static_cast<u32>(backlog);
    }
};
}

#endif // DSI_DSPBOUNDS_H
