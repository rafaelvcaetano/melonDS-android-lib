#include <oboe/Oboe.h>
#include "MelonDS.h"
#include "FileUtils.h"
#include "OboeCallback.h"
#include "../NDS.h"
#include "../GPU.h"
#include "../GPU3D.h"
#include "../GBACart.h"
#include "../SPU.h"
#include "../Platform.h"
#include "../Config.h"
#include <android/asset_manager.h>
#include <cstring>

u32* frameBuffer;
oboe::AudioStream *audioStream;

namespace MelonDSAndroid
{
    char* configDir;
    AAssetManager* assetManager;

    void setup(EmulatorConfiguration emulatorConfiguration, AAssetManager* androidAssetManager)
    {
        assetManager = androidAssetManager;

        frameBuffer = new u32[256 * 384 * 4];

        // TODO: Gotta find the correct sound setup
        oboe::AudioStreamBuilder streamBuilder;
        streamBuilder.setChannelCount(2);
        streamBuilder.setFramesPerCallback(1024);
        streamBuilder.setFormat(oboe::AudioFormat::I16);
        streamBuilder.setDirection(oboe::Direction::Output);
        streamBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        streamBuilder.setSharingMode(oboe::SharingMode::Exclusive);
        streamBuilder.setCallback(new OboeCallback());

        oboe::Result result = streamBuilder.openStream(&audioStream);
        if (result != oboe::Result::OK)
        {
            fprintf(stderr, "Failed to init audio stream");
        }

        // DS BIOS files are always required
        char* dsBios7 = joinPaths(emulatorConfiguration.dsConfigDir, "bios7.bin");
        char* dsBios9 = joinPaths(emulatorConfiguration.dsConfigDir, "bios9.bin");
        strcpy(Config::BIOS7Path, dsBios7);
        strcpy(Config::BIOS9Path, dsBios9);
        delete[] dsBios7;
        delete[] dsBios9;

        if (emulatorConfiguration.consoleType == 0) {
            configDir = emulatorConfiguration.dsConfigDir;
            strcpy(Config::FirmwarePath, "firmware.bin");
            NDS::SetConsoleType(0);
        } else {
            configDir = emulatorConfiguration.dsiConfigDir;
            strcpy(Config::DSiBIOS7Path, "bios7.bin");
            strcpy(Config::DSiBIOS9Path, "bios9.bin");
            strcpy(Config::DSiFirmwarePath, "firmware.bin");
            strcpy(Config::DSiNANDPath, "nand.bin");
            NDS::SetConsoleType(1);
        }

#ifdef JIT_ENABLED
        Config::JIT_Enable = emulatorConfiguration.useJit ? 1 : 0;
#endif

        NDS::Init();
        GPU::InitRenderer(0);
        GPU::SetRenderSettings(0, emulatorConfiguration.renderSettings);
    }

    void updateRendererConfiguration(GPU::RenderSettings renderSettings)
    {
        GPU::SetRenderSettings(0, renderSettings);
    }

    int loadRom(char* romPath, char* sramPath, bool loadDirect, bool loadGbaRom, char* gbaRom, char* gbaSram)
    {
        bool loaded = NDS::LoadROM(romPath, sramPath, loadDirect);
        if (!loaded)
            return 2;

        // Slot 2 is not supported in DSi
        if (loadGbaRom && NDS::ConsoleType == 0)
        {
            if (!NDS::LoadGBAROM(gbaRom, gbaSram))
                return 1;
        }

        return 0;
    }

    void start()
    {
        audioStream->requestStart();
        memset(frameBuffer, 0, 256 * 384 * 4);
    }

    u32 loop()
    {
        u32 nLines = NDS::RunFrame();

        int frontbuf = GPU::FrontBuffer;
        if (GPU::Framebuffer[frontbuf][0] && GPU::Framebuffer[frontbuf][1])
        {
            memcpy(frameBuffer, GPU::Framebuffer[frontbuf][0], 256 * 192 * 4);
            memcpy(&frameBuffer[256 * 192], GPU::Framebuffer[frontbuf][1], 256 * 192 * 4);
        }

        return nLines;
    }

    void pause() {
        audioStream->requestPause();
    }

    void resume() {
        audioStream->requestStart();
    }

    void copyFrameBuffer(void* dstBuffer)
    {
        memcpy(dstBuffer, frameBuffer, 256 * 384 * 4);
    }

    bool saveState(const char* path)
    {
        Savestate* savestate = new Savestate(path, true);
        if (savestate->Error)
        {
            delete savestate;
            return false;
        }
        else
        {
            NDS::DoSavestate(savestate);
            delete savestate;
            return true;
        }
    }

    bool loadState(const char* path)
    {
        bool success = true;
        char* backupPath = joinPaths(configDir, "backup.mln");

        Savestate* backup = new Savestate(backupPath, true);
        NDS::DoSavestate(backup);
        delete backup;

        Savestate* savestate = new Savestate(path, false);
        if (savestate->Error)
        {
            delete savestate;

            savestate = new Savestate(backupPath, false);
            success = false;
        }

        NDS::DoSavestate(savestate);
        delete savestate;

        // Delete backup file
        remove(backupPath);

        delete[] backupPath;

        return success;
    }

    void cleanup()
    {
        GBACart::Eject();
        NDS::DeInit();
        audioStream->requestStop();
        audioStream->close();
        free(audioStream);
        audioStream = NULL;
        assetManager = NULL;

        free(frameBuffer);
        frameBuffer = NULL;
    }
}

void Stop(bool internal)
{
}