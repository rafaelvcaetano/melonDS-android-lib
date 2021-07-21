#ifndef MELONDS_MELONDS_H
#define MELONDS_MELONDS_H

#include <list>
#include "AndroidFileHandler.h"
#include "../types.h"
#include "../GPU.h"
#include <android/asset_manager.h>

namespace MelonDSAndroid {
    typedef struct {
        char username[11];
        int language;
        int birthdayMonth;
        int birthdayDay;
        int favouriteColour;
        char message[27];
        bool randomizeMacAddress;
        u8 macAddress[6];
    } FirmwareConfiguration;

    typedef struct {
        bool userInternalFirmwareAndBios;
        char* dsBios7Path;
        char* dsBios9Path;
        char* dsFirmwarePath;
        char* dsiBios7Path;
        char* dsiBios9Path;
        char* dsiFirmwarePath;
        char* dsiNandPath;
        char* internalFilesDir;
        float fastForwardSpeedMultiplier;
        bool useJit;
        int consoleType;
        bool soundEnabled;
        int micSource;
        FirmwareConfiguration firmwareConfiguration;
        GPU::RenderSettings renderSettings;
    } EmulatorConfiguration;

    typedef struct {
        u32 codeLength;
        u32 code[2*64];
    } Cheat;

    typedef enum {
        ROM,
        FIRMWARE
    } RunMode;

    extern AAssetManager* assetManager;
    extern AndroidFileHandler* fileHandler;
    extern FirmwareConfiguration firmwareConfiguration;

    extern void setup(EmulatorConfiguration emulatorConfiguration, AAssetManager* androidAssetManager, AndroidFileHandler* androidFileHandler, u32* textureBufferPointer);
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
    extern bool reset();
    extern void updateMic();
    extern bool saveState(const char* path);
    extern bool loadState(const char* path);
    extern void cleanup();
}

#endif //MELONDS_MELONDS_H
