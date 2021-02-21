#include <oboe/Oboe.h>
#include "MelonDS.h"
#include "FileUtils.h"
#include "OboeCallback.h"
#include "MicInputOboeCallback.h"
#include "AndroidARCodeFile.h"
#include "../NDS.h"
#include "../GPU.h"
#include "../GPU3D.h"
#include "../GBACart.h"
#include "../SPU.h"
#include "../Platform.h"
#include "../Config.h"
#include "../AREngine.h"
#include "SharedConfig.h"
#include "PlatformConfig.h"
#include "FrontendUtil.h"
#include <android/asset_manager.h>
#include <cstring>

#define MIC_BUFFER_SIZE 2048

u32* frameBuffer;
oboe::AudioStream *audioStream;
oboe::AudioStream *micInputStream;
OboeCallback *outputCallback;
MicInputOboeCallback *micInputCallback;
AndroidARCodeFile *arCodeFile;

namespace MelonDSAndroid
{
    char* configDir;
    int micInputType;
    AAssetManager* assetManager;
    FirmwareConfiguration firmwareConfiguration;

    // Variables used to keep the current state so that emulation can be reset
    char* currentRomPath = NULL;
    char* currentSramPath = NULL;
    char* currentGbaRomPath = NULL;
    char* currentGbaSramPath = NULL;
    bool currentLoadGbaRom;
    bool currentLoadDirect;
    RunMode currentRunMode;

    void setupAudioOutputStream();
    void setupMicInputStream();
    void copyString(char** dest, const char* source);

    void setup(EmulatorConfiguration emulatorConfiguration, AAssetManager* androidAssetManager) {
        assetManager = androidAssetManager;
        firmwareConfiguration = emulatorConfiguration.firmwareConfiguration;

        frameBuffer = new u32[256 * 384 * 4];
        micInputType = emulatorConfiguration.micSource;

        if (emulatorConfiguration.soundEnabled) {
            setupAudioOutputStream();
        }

        if (micInputType == 2) {
            setupMicInputStream();
        }

        if (emulatorConfiguration.consoleType == 0 && emulatorConfiguration.userInternalFirmwareAndBios) {
            strcpy(Config::BIOS7Path, "?bios/drastic_bios_arm7.bin");
            strcpy(Config::BIOS9Path, "?bios/drastic_bios_arm9.bin");
        } else {
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
                Config::ConsoleType = 0;
                NDS::SetConsoleType(0);
            } else {
                configDir = emulatorConfiguration.dsiConfigDir;
                strcpy(Config::DSiBIOS7Path, "bios7.bin");
                strcpy(Config::DSiBIOS9Path, "bios9.bin");
                strcpy(Config::DSiFirmwarePath, "firmware.bin");
                strcpy(Config::DSiNANDPath, "nand.bin");
                Config::ConsoleType = 1;
                NDS::SetConsoleType(1);
            }
        }

#ifdef JIT_ENABLED
        Config::JIT_Enable = emulatorConfiguration.useJit ? 1 : 0;
#endif

        Config::UseInternalFirmware = emulatorConfiguration.userInternalFirmwareAndBios;
        Config::RandomizeMAC = 1;
        Config::SocketBindAnyAddr = 1;

        NDS::Init();
        GPU::InitRenderer(0);
        GPU::SetRenderSettings(0, emulatorConfiguration.renderSettings);
    }

    void setCodeList(std::list<Cheat> cheats)
    {
        if (arCodeFile == NULL) {
            arCodeFile = new AndroidARCodeFile();
            AREngine::SetCodeFile(arCodeFile);
        }

        ARCodeList codeList;

        for (std::list<Cheat>::iterator it = cheats.begin(); it != cheats.end(); it++)
        {
            Cheat& cheat = *it;

            ARCode code = {
                    .Enabled = true,
                    .CodeLen = cheat.codeLength
            };
            memcpy(code.Code, cheat.code, sizeof(code.Code));

            codeList.push_back(code);
        }

        arCodeFile->updateCodeList(codeList);
    }

    void updateEmulatorConfiguration(EmulatorConfiguration emulatorConfiguration) {
        int oldMicSource = micInputType;

        GPU::SetRenderSettings(0, emulatorConfiguration.renderSettings);
        micInputType = emulatorConfiguration.micSource;

        if (emulatorConfiguration.soundEnabled) {
            if (audioStream == NULL) {
                setupAudioOutputStream();
            }
        } else if (audioStream != NULL) {
            audioStream->requestStop();
            audioStream->close();
            delete audioStream;
            delete outputCallback;
            audioStream = NULL;
            outputCallback = NULL;
        }

        if (oldMicSource == 2 && micInputType != 2) {
            // No longer using device mic. Destroy stream
            if (micInputStream != NULL) {
                micInputStream->requestStop();
                micInputStream->close();
                delete micInputStream;
                delete micInputCallback;
                micInputStream = NULL;
                micInputCallback = NULL;
            }
        } else if (oldMicSource != 2 && micInputType == 2) {
            // Now using device mic. Setup stream
            setupMicInputStream();
        }
    }

    int loadRom(char* romPath, char* sramPath, bool loadDirect, bool loadGbaRom, char* gbaRom, char* gbaSram)
    {
        copyString(&currentRomPath, romPath);
        copyString(&currentSramPath, sramPath);
        copyString(&currentGbaRomPath, gbaRom);
        copyString(&currentGbaSramPath, gbaSram);
        currentLoadDirect = loadDirect;
        currentLoadGbaRom = loadGbaRom;
        currentRunMode = ROM;

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

    int bootFirmware()
    {
        currentRunMode = FIRMWARE;
        return Frontend::LoadBIOS();
    }

    void start()
    {
        if (audioStream != NULL)
            audioStream->requestStart();

        if (micInputStream != NULL)
            micInputStream->requestStart();

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
        if (audioStream != NULL)
            audioStream->requestPause();

        if (micInputStream != NULL)
            micInputStream->requestPause();
    }

    void resume() {
        if (audioStream != NULL)
            audioStream->requestStart();

        if (micInputStream != NULL)
            micInputStream->requestStart();
    }

    bool reset()
    {
        if (currentRunMode == ROM) {
            int result = loadRom(currentRomPath, currentSramPath, currentLoadDirect, currentLoadGbaRom, currentGbaRomPath, currentGbaSramPath);
            if (result != 2 && arCodeFile != NULL) {
                AREngine::SetCodeFile(arCodeFile);
            }

            return result != 2;
        } else {
            int result = bootFirmware();
            return result == 0;
        }
    }

    void copyFrameBuffer(void* dstBuffer)
    {
        memcpy(dstBuffer, frameBuffer, 256 * 384 * 4);
    }

    void updateMic()
    {
        switch (micInputType)
        {
            case 0: // no mic
                Frontend::Mic_FeedSilence();
                break;
            case 1: // white noise
                Frontend::Mic_FeedNoise();
                break;
            case 2: // host mic
                Frontend::Mic_FeedExternalBuffer();
                break;
        }
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
        GPU::DeInitRenderer();
        NDS::DeInit();

        free(currentRomPath);
        free(currentSramPath);
        free(currentGbaRomPath);
        free(currentGbaSramPath);
        currentRomPath = NULL;
        currentSramPath = NULL;
        currentGbaRomPath = NULL;
        currentGbaSramPath = NULL;

        if (audioStream != NULL) {
            audioStream->requestStop();
            audioStream->close();
            delete audioStream;
            delete outputCallback;
            audioStream = NULL;
            outputCallback = NULL;
        }

        if (micInputStream != NULL) {
            micInputStream->requestStop();
            micInputStream->close();
            delete micInputStream;
            delete micInputCallback;
            micInputStream = NULL;
            micInputCallback = NULL;
            Frontend::Mic_SetExternalBuffer(NULL, 0);
        }

        if (arCodeFile != NULL) {
            delete arCodeFile;
            arCodeFile = NULL;
        }

        assetManager = NULL;

        free(frameBuffer);
        frameBuffer = NULL;
    }

    void setupAudioOutputStream()
    {
        outputCallback = new OboeCallback();
        oboe::AudioStreamBuilder streamBuilder;
        streamBuilder.setChannelCount(2);
        streamBuilder.setFramesPerCallback(1024);
        streamBuilder.setSampleRate(48000);
        streamBuilder.setFormat(oboe::AudioFormat::I16);
        streamBuilder.setDirection(oboe::Direction::Output);
        streamBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        streamBuilder.setSharingMode(oboe::SharingMode::Shared);
        streamBuilder.setCallback(outputCallback);

        oboe::Result result = streamBuilder.openStream(&audioStream);
        if (result != oboe::Result::OK) {
            fprintf(stderr, "Failed to init audio stream");
            delete outputCallback;
            outputCallback = NULL;
        } else {
            Frontend::Init_Audio(audioStream->getSampleRate());
        }
    }

    void setupMicInputStream()
    {
        micInputCallback = new MicInputOboeCallback(MIC_BUFFER_SIZE);
        oboe::AudioStreamBuilder micStreamBuilder;
        micStreamBuilder.setChannelCount(1);
        micStreamBuilder.setFramesPerCallback(1024);
        micStreamBuilder.setSampleRate(44100);
        micStreamBuilder.setFormat(oboe::AudioFormat::I16);
        micStreamBuilder.setDirection(oboe::Direction::Input);
        micStreamBuilder.setInputPreset(oboe::InputPreset::Generic);
        micStreamBuilder.setPerformanceMode(oboe::PerformanceMode::PowerSaving);
        micStreamBuilder.setSharingMode(oboe::SharingMode::Exclusive);
        micStreamBuilder.setCallback(micInputCallback);

        oboe::Result micResult = micStreamBuilder.openStream(&micInputStream);
        if (micResult != oboe::Result::OK) {
            micInputType = 1;
            fprintf(stderr, "Failed to init mic audio stream");
            delete micInputCallback;
            micInputCallback = NULL;
        } else {
            Frontend::Mic_SetExternalBuffer(micInputCallback->buffer, MIC_BUFFER_SIZE);
        }
    }

    void copyString(char** dest, const char* source)
    {
        if (source == NULL) {
            *dest = NULL;
            return;
        }

        int length = strlen(source);
        *dest = (char*) malloc(length + 1);
        strcpy(*dest, source);
    }
}

