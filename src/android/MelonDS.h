#ifndef MELONDS_MELONDS_H
#define MELONDS_MELONDS_H

#include <list>
#include <vector>
#include "AndroidFileHandler.h"
#include "AndroidCameraHandler.h"
#include "Configuration.h"
#include "RewindManager.h"
#include "RomGbaSlotConfig.h"
#include "retroachievements/RAAchievement.h"
#include "retroachievements/RACallback.h"
#include "renderer/FrameQueue.h"
#include "types.h"
#include "../GPU.h"
#include <android/asset_manager.h>

using namespace melonDS;

namespace MelonDSAndroid {
    typedef struct {
        std::vector<u32> code;
    } Cheat;

    typedef enum {
        ROM,
        FIRMWARE
    } RunMode;

    extern OpenGLContext *openGlContext;
    extern AndroidFileHandler* fileHandler;
    extern AndroidCameraHandler* cameraHandler;
    extern std::string internalFilesDir;

    extern void setConfiguration(EmulatorConfiguration emulatorConfiguration);
    extern void setup(AndroidCameraHandler* androidCameraHandler, RetroAchievements::RACallback* raCallback, u32* screenshotBufferPointer, int instanceId);
    extern void setCodeList(std::list<Cheat> cheats);
    extern void setupAchievements(std::list<RetroAchievements::RAAchievement> achievements, std::optional<std::string> richPresenceScript);
    extern void unloadAchievements(std::list<RetroAchievements::RAAchievement> achievements);
    extern std::string getRichPresenceStatus();
    extern void updateEmulatorConfiguration(std::unique_ptr<EmulatorConfiguration> emulatorConfiguration);

    /**
     * Loads the NDS ROM and, optionally, the GBA ROM.
     *
     * @param romPath The path to the NDS rom
     * @param sramPath The path to the rom's SRAM file
     * @param gbaSlotConfig The config to be used for the GBA slot
     * @return The load result. 0 if everything was loaded successfully, 1 if the NDS ROM was loaded but the GBA ROM
     * failed to load, 2 if the NDS ROM failed to load
     */
    extern int loadRom(std::string romPath, std::string sramPath, RomGbaSlotConfig* gbaSlotConfig);
    extern int bootFirmware();
    extern void touchScreen(u16 x, u16 y);
    extern void releaseScreen();
    extern void pressKey(u32 key);
    extern void releaseKey(u32 key);
    extern void start();
    extern u32 loop();
    extern Frame* getPresentationFrame();
    extern void pause();
    extern void resume();
    extern void reset();
    extern void enableMic();
    extern void disableMic();
    extern void updateMic();
    extern bool saveState(const char* path);
    extern bool loadState(const char* path);
    extern bool loadRewindState(melonDS::RewindSaveState rewindSaveState);
    extern RewindWindow getRewindWindow();
    extern void stop();
    extern void cleanup();
}

#endif //MELONDS_MELONDS_H
