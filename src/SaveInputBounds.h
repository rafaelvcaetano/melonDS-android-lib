#ifndef SAVEINPUTBOUNDS_H
#define SAVEINPUTBOUNDS_H

#include "types.h"

namespace melonDS
{
struct SaveInputBounds
{
    static constexpr u64 MaxNDSSaveLength = 32ULL * 1024 * 1024;

    static constexpr bool NDSLengthValid(u64 length)
    {
        return length <= MaxNDSSaveLength;
    }

    static constexpr bool GBALengthValid(u64 length)
    {
        return length == 0 || length == 512 || length == 8192 ||
            length == 32768 || length == 65536 || length == 131072 ||
            length == 131088;
    }
};
}

#endif // SAVEINPUTBOUNDS_H
