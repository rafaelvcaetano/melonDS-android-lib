#ifndef ROMGBASLOTCONFIG_H
#define ROMGBASLOTCONFIG_H

#include <string>

namespace MelonDSAndroid
{
    enum RomGbaSlotConfigType {
        NONE = 0,
        GBA_ROM = 1,
        MEMORY_EXPANSION = 2,
        RUMBLE_PAK = 3,
    } ;

    struct RomGbaSlotConfig {
        RomGbaSlotConfigType type;
    };

    struct RomGbaSlotConfigNone {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::NONE };
    };

    struct RomGbaSlotConfigGbaRom {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::GBA_ROM };
        std::string romPath;
        std::string savePath;
    };

    struct RomGbaSlotConfigMemoryExpansion {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::MEMORY_EXPANSION };
    };

    struct RomGbaSlotConfigRumblePak {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::RUMBLE_PAK };
    };
}

#endif //ROMGBASLOTCONFIG_H
