#ifndef DSISUPPORT_H
#define DSISUPPORT_H

#include <DSi.h>

namespace MelonDSAndroid
{
namespace DSiSupport
{

struct DSiAutoLoad
{
    uint8_t ID[4]; // "TLNC"
    uint8_t Unknown1; // "usually 01h"
    uint8_t Length; // starting from PrevTitleId
    uint16_t CRC16; // covering Length bytes ("18h=norm")
    uint8_t PrevTitleID[8]; // can be 0 ("anonymous")
    uint8_t NewTitleID[8];
    uint32_t Flags; // bit 0: is valid, bit 1-3: boot type ("01h=Cartridge, 02h=Landing, 03h=DSiware"), other bits unknown/unused
    uint32_t Unused1; // this part is typically still checksummed
    uint8_t Unused2[0xE0]; // this part isn't checksummed, but is 0 filled on erasing autoload data
};

void SetupDSiDirectBoot(melonDS::DSi *dsi);

}
}

#endif //DSISUPPORT_H
