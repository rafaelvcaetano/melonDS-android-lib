#include <cstring>
#include <utility>
#include <android/asset_manager.h>
#include <oboe/Oboe.h>
#include "EmulatorArgsBuilder.h"
#include "MelonDS.h"
#include "OboeCallback.h"
#include "MelonAudioStreamErrorCallback.h"
#include "MicInputOboeCallback.h"
#include "OpenGLContext.h"
#include "mic_blow.h"
#include "NDS.h"
#include "GPU.h"
#include "GPU3D.h"
#include "GBACart.h"
#include "SPU.h"
#include "Platform.h"
#include "Savestate.h"
#include "MelonInstance.h"
#include "RewindManager.h"
#include "ROMManager.h"
#include "MPInterface.h"
#include "AndroidCameraHandler.h"
#include "renderer/ScreenshotRenderer.h"
#include "renderer/FrameQueue.h"
#include "retroachievements/RetroAchievementsManager.h"
#include "retroachievements/RACallback.h"
#include "net/Net.h"
#include "net/Net_Slirp.h"

#define MIC_BUFFER_SIZE 2048

std::shared_ptr<oboe::AudioStream> audioStream;
oboe::AudioStreamErrorCallback *audioStreamErrorCallback;
std::shared_ptr<oboe::AudioStream> micInputStream;
OboeCallback *outputCallback;
MicInputOboeCallback *micInputCallback;

namespace MelonDSAndroid
{
    int actualMicSource = 0;
    bool isMicInputEnabled = true;
    OpenGLContext *openGlContext;
    AndroidFileHandler* fileHandler;
    AndroidCameraHandler* cameraHandler;
    std::string internalFilesDir;
    std::shared_ptr<EmulatorConfiguration> currentConfiguration;
    std::shared_ptr<Net> net;

    std::shared_ptr<MelonInstance> instance;

    void setupAudioOutputStream(int audioLatency, int volume);
    void cleanupAudioOutputStream();
    void setupMicInputStream();
    void cleanupMicInputStream();
    void resetAudioOutputStream();
    bool setupOpenGlContext();
    void cleanupOpenGlContext();

    /**
     * Used to set the emulator's initial configuration, before boot. To update the configuration during runtime, use @updateEmulatorConfiguration.
     *
     * @param emulatorConfiguration The emulator configuration during the next emulator run
     */
    void setConfiguration(EmulatorConfiguration emulatorConfiguration) {
        currentConfiguration = std::make_shared<EmulatorConfiguration>(std::move(emulatorConfiguration));
        internalFilesDir = currentConfiguration->internalFilesDir;
        actualMicSource = currentConfiguration->micSource;
        isMicInputEnabled = true;

        net = std::make_shared<Net>();
        net->SetDriver(std::make_unique<Net_Slirp>([](const u8* data, int len) {
            net->RXEnqueue(data, len);
        }));
    }

    void setup(AndroidCameraHandler* androidCameraHandler, RetroAchievements::RACallback* raCallback, u32* screenshotBufferPointer, int instanceId)
    {
        cameraHandler = androidCameraHandler;
        RetroAchievements::RetroAchievementsManager::AchievementsCallback = raCallback;

        auto instanceArgs = BuildArgsFromConfiguration(*currentConfiguration, instanceId);
        if (!instanceArgs.has_value())
        {
            // TODO: Handle this somehow?
            instance = nullptr;
            return;
        }
        instance = std::make_shared<MelonInstance>(
            instanceId,
            currentConfiguration,
            std::move(instanceArgs.value()),
            net,
            ScreenshotRenderer(screenshotBufferPointer),
            currentConfiguration->consoleType
        );

        if (currentConfiguration->soundEnabled)
        {
            setupAudioOutputStream(currentConfiguration->audioLatency, currentConfiguration->volume);
        }
        if (currentConfiguration->micSource == 2)
        {
            setupMicInputStream();
        }
    }

    void setCodeList(std::list<Cheat> cheats)
    {
        instance->loadCheats(std::move(cheats));
    }

    void setupAchievements(
        std::list<RetroAchievements::RAAchievement> achievements,
        std::list<RetroAchievements::RALeaderboard> leaderboards,
        std::optional<std::string> richPresenceScript
    )
    {
        instance->setupAchievements(std::move(achievements), std::move(leaderboards), std::move(richPresenceScript));
    }

    void unloadRetroAchievementsData()
    {
        instance->unloadRetroAchievementsData();
    }

    std::string getRichPresenceStatus()
    {
        return instance->getRichPresenceStatus();
    }

    /**
     * Used to update the emulator's configuration during runtime. Will only update the configurations that can actually change during runtime without causing issues,
     *
     * @param emulatorConfiguration The new emulator configuration
     */
    void updateEmulatorConfiguration(std::unique_ptr<EmulatorConfiguration> emulatorConfiguration) {
        std::shared_ptr<EmulatorConfiguration> sharedConfig = std::move(emulatorConfiguration);
        instance->updateConfiguration(sharedConfig);

        if (sharedConfig->soundEnabled && currentConfiguration->volume > 0) {
            if (!audioStream) {
                setupAudioOutputStream(sharedConfig->audioLatency, sharedConfig->volume);
            } else if (currentConfiguration->audioLatency != sharedConfig->audioLatency || currentConfiguration->volume != sharedConfig->volume) {
                // Recreate audio stream with new settings
                cleanupAudioOutputStream();
                setupAudioOutputStream(sharedConfig->audioLatency, sharedConfig->volume);
            }
        } else if (audioStream) {
            cleanupAudioOutputStream();
        }

        int oldMicSource = actualMicSource;
        actualMicSource = sharedConfig->micSource;

        if (oldMicSource == 2 && sharedConfig->micSource != 2) {
            // No longer using device mic. Destroy stream
            cleanupMicInputStream();
        } else if (oldMicSource != 2 && sharedConfig->micSource == 2) {
            // Now using device mic. Setup stream
            setupMicInputStream();
        }

        currentConfiguration = sharedConfig;
    }

    int loadRom(std::string romPath, std::string sramPath, RomGbaSlotConfig* gbaSlotConfig)
    {
        if (!instance->loadRom(std::move(romPath), std::move(sramPath)))
            return 2;

        if (gbaSlotConfig->type == GBA_ROM)
        {
            RomGbaSlotConfigGbaRom* gbaRomConfig = (RomGbaSlotConfigGbaRom*) gbaSlotConfig;
            if (!instance->loadGbaRom(gbaRomConfig->romPath, gbaRomConfig->savePath))
                return 1;
        }
        else if (gbaSlotConfig->type == MEMORY_EXPANSION)
        {
            instance->loadGbaMemoryExpansion();
        }

        return 0;
    }

    int bootFirmware()
    {
        // TODO: Maybe validate BIOS and firmware?
        if (instance->bootFirmware())
            return ROMManager::SUCCESS;
        else
            return ROMManager::FIRMWARE_NOT_BOOTABLE;
    }

    void touchScreen(u16 x, u16 y)
    {
        if (instance)
            instance->touchScreen(x, y);
    }

    void releaseScreen()
    {
        if (instance)
            instance->releaseScreen();
    }

    void pressKey(u32 key)
    {
        if (instance)
            instance->pressKey(key);
    }

    void releaseKey(u32 key)
    {
        if (instance)
            instance->releaseKey(key);
    }

    void start()
    {
        if (audioStream)
            audioStream->requestStart();

        if (micInputStream && isMicInputEnabled)
            micInputStream->requestStart();

        setupOpenGlContext();

        instance->start();
    }

    u32 loop()
    {
        MPInterface::Get().Process();
        return instance->runFrame();
    }

    Frame* getPresentationFrame()
    {
        if (!instance)
            return nullptr;

        return instance->getPresentationFrame();
    }

    void pause()
    {
        if (audioStream)
            audioStream->requestPause();

        if (micInputStream && isMicInputEnabled)
            micInputStream->requestStop();
    }

    void resume()
    {
        if (audioStream)
            audioStream->requestStart();

        if (micInputStream && isMicInputEnabled)
            micInputStream->requestStart();
    }

    void reset()
    {
        instance->reset();
    }

    void enableMic()
    {
        isMicInputEnabled = true;
        if (actualMicSource == 2 && micInputStream)
        {
            micInputStream->requestStart();
        }
    }

    void disableMic()
    {
        isMicInputEnabled = false;
        if (actualMicSource == 2 && micInputStream)
        {
            micInputStream->requestStop();
        }
    }

    void updateMic()
    {
        if (!isMicInputEnabled)
        {
            instance->feedMicAudio(nullptr, 0);
            return;
        }

        switch (actualMicSource)
        {
            case 0: // no mic
                instance->feedMicAudio(nullptr, 0);
                break;
            case 1: // white noise
            {
                int sample_len = sizeof(mic_blow) / sizeof(u16);
                static int sample_pos = 0;

                s16 tmp[735];

                for (int i = 0; i < 735; i++)
                {
                    tmp[i] = mic_blow[sample_pos] ^ 0x8000;
                    sample_pos++;
                    if (sample_pos >= sample_len)
                        sample_pos = 0;
                }

                instance->feedMicAudio(tmp, 735);
                break;
            }
            case 2: // host mic
                instance->feedMicAudio(micInputCallback->buffer, 735);
                break;
        }
    }

    bool saveState(const char* path)
    {
        Platform::FileHandle* saveStateFile = Platform::OpenFile(path, Platform::FileMode::Write);

        if (!saveStateFile)
            return false;

        Savestate state;
        if (state.Error)
        {
            Platform::CloseFile(saveStateFile);
            return false;
        }

        instance->saveState(&state);

        if (state.Error)
        {
            Platform::CloseFile(saveStateFile);
            return false;
        }

        if (Platform::FileWrite(state.Buffer(), state.Length(), 1, saveStateFile) == 0)
        {
            Platform::Log(Platform::Error, "Failed to write %d-byte savestate to %s\n", state.Length(), path);
            Platform::CloseFile(saveStateFile);
            return false;
        }

        Platform::CloseFile(saveStateFile);
        return true;
    }

    bool loadState(const char* path)
    {
        auto saveStateFile = Platform::OpenFile(path, Platform::FileMode::Read);
        if (!saveStateFile)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to open state file \"%s\"\n", path);
            return false;
        }

        std::unique_ptr<Savestate> backup = std::make_unique<Savestate>(Savestate::DEFAULT_SIZE);
        if (backup->Error)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to allocate memory for state backup\n");
            Platform::CloseFile(saveStateFile);
            return false;
        }

        if (!instance->saveState(backup.get()) || backup->Error)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to back up state, aborting load (from \"%s\")\n", path);
            Platform::CloseFile(saveStateFile);
            return false;
        }

        size_t size = Platform::FileLength(saveStateFile);

        // Allocate exactly as much memory as we need for the savestate
        std::vector<u8> buffer(size);
        if (Platform::FileRead(buffer.data(), size, 1, saveStateFile) == 0)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to read %u-byte state file \"%s\"\n", size, path);
            Platform::CloseFile(saveStateFile);
            return false;
        }
        Platform::CloseFile(saveStateFile);

        std::unique_ptr<Savestate> state = std::make_unique<Savestate>(buffer.data(), size, false);

        if (!instance->loadState(state.get()) || state->Error)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to load state file \"%s\" into emulator\n", path);
            // Restore backup
            if (!instance->loadState(backup.get()) || state->Error)
                Platform::Log(Platform::LogLevel::Error, "Failed to load backup state\n", path);
            else
                Platform::Log(Platform::LogLevel::Info, "Backup state loaded\n", path);

            return false;
        }
        return true;
    }

    bool loadRewindState(melonDS::RewindSaveState rewindSaveState)
    {
        std::unique_ptr<Savestate> backup = std::make_unique<Savestate>(Savestate::DEFAULT_SIZE);
        if (backup->Error)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to allocate memory for state backup");
            return false;
        }

        if (!instance->saveState(backup.get()) || backup->Error)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to back up state, aborting rewind state load");
            return false;
        }

        bool result = instance->loadRewindState(rewindSaveState);
        if (!result)
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to load rewind state");
            // Restore backup
            if (!instance->loadState(backup.get()) || backup->Error)
                Platform::Log(Platform::LogLevel::Error, "Failed to load backup state");
            else
                Platform::Log(Platform::LogLevel::Info, "Backup state loaded");
        }

        return result;
    }

    RewindWindow getRewindWindow()
    {
        return instance->getRewindWindow();
    }

    void stop()
    {
        instance->stop();
        cleanupOpenGlContext();
    }

    void cleanup()
    {
        cleanupAudioOutputStream();
        cleanupMicInputStream();

        instance = nullptr;
    }

    void setupAudioOutputStream(int audioLatency, int volume)
    {
        oboe::PerformanceMode performanceMode;
        switch (audioLatency) {
            case 0:
                performanceMode = oboe::PerformanceMode::LowLatency;
                break;
            case 1:
                performanceMode = oboe::PerformanceMode::None;
                break;
            case 2:
                performanceMode = oboe::PerformanceMode::PowerSaving;
                break;
            default:
                performanceMode = oboe::PerformanceMode::None;
        }

        outputCallback = new OboeCallback(volume);
        outputCallback->activeInstance = instance;

        audioStreamErrorCallback = new MelonAudioStreamErrorCallback(resetAudioOutputStream);
        oboe::AudioStreamBuilder streamBuilder;
        streamBuilder.setChannelCount(2);
        streamBuilder.setFramesPerCallback(1024);
        streamBuilder.setSampleRate(48000);
        streamBuilder.setFormat(oboe::AudioFormat::I16);
        streamBuilder.setDirection(oboe::Direction::Output);
        streamBuilder.setPerformanceMode(performanceMode);
        streamBuilder.setSharingMode(oboe::SharingMode::Shared);
        streamBuilder.setCallback(outputCallback);
        streamBuilder.setErrorCallback(audioStreamErrorCallback);

        oboe::Result result = streamBuilder.openStream(audioStream);
        if (result != oboe::Result::OK) {
            fprintf(stderr, "Failed to init audio stream");
            delete outputCallback;
            delete audioStreamErrorCallback;
            outputCallback = nullptr;
            audioStreamErrorCallback = nullptr;
        }
    }

    void cleanupAudioOutputStream()
    {
        if (audioStream) {
            if (audioStream->getState() < oboe::StreamState::Closing) {
                audioStream->requestStop();
                audioStream->close();
            }
            delete outputCallback;
            delete audioStreamErrorCallback;
            audioStream = nullptr;
            outputCallback = nullptr;
            audioStreamErrorCallback = nullptr;
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

        oboe::Result micResult = micStreamBuilder.openStream(micInputStream);
        if (micResult != oboe::Result::OK) {
            actualMicSource = 1;
            fprintf(stderr, "Failed to init mic audio stream");
            delete micInputCallback;
            micInputCallback = nullptr;
        }
    }

    void cleanupMicInputStream()
    {
        if (micInputStream) {
            micInputStream->requestStop();
            micInputStream->close();
            delete micInputCallback;
            micInputStream = nullptr;
            micInputCallback = nullptr;
        }
    }

    void resetAudioOutputStream()
    {
        cleanupAudioOutputStream();
        setupAudioOutputStream(currentConfiguration->audioLatency, currentConfiguration->volume);
        if (audioStream) {
            audioStream->requestStart();
        }
    }

    bool setupOpenGlContext()
    {
        if (openGlContext == nullptr)
            return false;

        if (!openGlContext->Use())
        {
            Platform::Log(Platform::LogLevel::Error, "Failed to use OpenGL context");
            return false;
        }

        return true;
    }

    void cleanupOpenGlContext()
    {
        if (openGlContext == nullptr)
            return;

        openGlContext->Release();
    }
}

