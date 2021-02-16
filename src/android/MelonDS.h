#ifndef MELONDS_MELONDS_H
#define MELONDS_MELONDS_H

#include <list>
#include "../types.h"
#include "../GPU.h"
#include <android/asset_manager.h>

namespace MelonDSAndroid {
    typedef struct {
        char* dsConfigDir;
        char* dsiConfigDir;
        float fastForwardSpeedMultiplier;
        bool useJit;
        int consoleType;
        int micSource;
        GPU::RenderSettings renderSettings;
    } EmulatorConfiguration;

    typedef struct {
        u32 codeLength;
        u32 code[2*64];
    } Cheat;

    extern char* configDir;
    extern AAssetManager* assetManager;

    extern void setup(EmulatorConfiguration emulatorConfiguration, AAssetManager* androidAssetManager);
    extern void setCodeList(std::list<Cheat> cheats);
    extern void updateEmulatorConfiguration(EmulatorConfiguration emulatorConfiguration);

    /**
     * Loads the NDS ROM and, optionally, the GBA ROM.
     *
     * @param romPath The path to the NDS rom
     * @param sramPath The path to the rom's SRAM file
     * @param loadDirect If the game should be booted directly
     * @param loadGbaRom If a GBA ROM must also be loaded
     * @param gbaRom The path to the GBA ROM
     * @param gbaSram The path to the GBA rom's SRAM file
     * @return The load result. 0 if everything was loaded successfully, 1 if the NDS ROM was loaded but the GBA ROM
     * failed to load, 2 if the NDS ROM failed to load
     */
    extern int loadRom(char* romPath, char* sramPath, bool loadDirect, bool loadGbaRom, char* gbaRom, char* gbaSram);
    extern int bootFirmware();
    extern void start();
    extern u32 loop();
    extern void pause();
    extern void resume();
    extern void copyFrameBuffer(void* dstBuffer);
    extern void updateMic();
    extern bool saveState(const char* path);
    extern bool loadState(const char* path);
    extern void cleanup();
}

#endif //MELONDS_MELONDS_H
