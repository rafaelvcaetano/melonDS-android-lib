#ifndef WIFITXBOUNDS_H
#define WIFITXBOUNDS_H

#include "types.h"

namespace melonDS
{
struct WifiTxBounds
{
    static constexpr u32 RamSize = 0x2000;
    static constexpr u32 TxHeaderSize = 12;

    static constexpr bool DescriptorFits(u32 address)
    {
        return address <= RamSize - TxHeaderSize;
    }

    static constexpr bool FrameFits(u32 address, u32 frameLength)
    {
        return DescriptorFits(address) && frameLength <= RamSize - TxHeaderSize - address;
    }

    static constexpr bool ContainsFrameBytes(u32 frameLength, u32 offset, u32 count)
    {
        return offset <= frameLength && count <= frameLength - offset;
    }

    static constexpr bool PreparedSlotCanStart(bool valid)
    {
        return valid;
    }

    static constexpr u16 BusyMask(int slot)
    {
        if (slot == 5) return 0x0080;
        return slot >= 0 && slot < 5 ? static_cast<u16>(1U << slot) : 0;
    }

    static constexpr bool RuntimeSlotValid(int slot, bool descriptorValid, u8 phase)
    {
        if (!descriptorValid)
            return slot == 5 && (phase == 10 || phase == 11);
        if (slot == 1)
            return phase <= 3 || phase == 13;
        if (slot >= 0 && slot <= 5)
            return phase <= 1;
        return false;
    }
};
}

#endif // WIFITXBOUNDS_H
