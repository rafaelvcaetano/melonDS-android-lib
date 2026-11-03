#ifndef DSITITLEDATABOUNDS_H
#define DSITITLEDATABOUNDS_H

#include "types.h"

namespace melonDS::DSiTitleDataBounds
{

inline bool ValidSaveLength(u32 size)
{
    return size >= 512 && (size & 0x1FF) == 0 && (size >> 9) <= 0xFFFF;
}

inline bool ExpectedLength(int type, u32 publicSize, u32 privateSize, u8 appFlags, u64& length)
{
    switch (type)
    {
    case 0:
        length = publicSize;
        return ValidSaveLength(publicSize);
    case 1:
        length = privateSize;
        return ValidSaveLength(privateSize);
    case 2:
        length = 0x4000;
        return (appFlags & 0x04) != 0;
    default:
        return false;
    }
}

}

#endif
