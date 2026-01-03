#include "DSiSupport.h"

// Implementation adapted from https://github.com/JesseTG/melonds-ds/blob/main/src/libretro/console/dsi.cpp

constexpr size_t DSI_AUTOLOAD_OFFSET = 0x300;
// unknown bit, seems to be required to boot into games (errors otherwise?)
constexpr uint32_t UNKNOWN_BOOT_BIT = (1 << 4);

void MelonDSAndroid::DSiSupport::SetupDSiDirectBoot(melonDS::DSi* dsi)
{
    auto* bptwl = dsi->I2C.GetBPTWL();

    bptwl->SetBootFlag(true);

    // setup "auto-load" feature
    auto cart = dsi->GetNDSCart();
    auto header = cart->GetHeader();
    DSiAutoLoad autoLoad {};
    memcpy(autoLoad.ID, "TLNC", sizeof(autoLoad.ID));
    autoLoad.Unknown1 = 0x01;
    autoLoad.Length = 0x18;
    memcpy(autoLoad.NewTitleID, &header.DSiTitleIDLow, sizeof(autoLoad.NewTitleID));
    // Copy header.DSiTitleIDLow and header.DSiTitleIDHigh to autoLoad.NewTitleID

    autoLoad.Flags |= (0x03 << 1) | 0x01 | UNKNOWN_BOOT_BIT;
    autoLoad.Flags |= (1 << 4);
    autoLoad.CRC16 = melonDS::CRC16((uint8_t*) &autoLoad.PrevTitleID, autoLoad.Length, 0xFFFF);
    memcpy(&dsi->MainRAM[DSI_AUTOLOAD_OFFSET], &autoLoad, sizeof(autoLoad));
}