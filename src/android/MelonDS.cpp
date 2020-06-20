#include <oboe/Oboe.h>
#include "MelonDS.h"
#include "../NDS.h"
#include "../GPU.h"
#include "../GPU3D.h"
#include "../SPU.h"
#include "../Platform.h"
#include "OboeCallback.h"
#include <android/asset_manager.h>

u32* frameBuffer;
oboe::AudioStream *audioStream;

namespace MelonDSAndroid
{
    char* configDir;
    AAssetManager* assetManager;

    void setup(char* configDirPath, AAssetManager* androidAssetManager)
    {
        configDir = configDirPath;
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

        NDS::Init();
        GPU3D::InitRenderer(false);
    }

    int loadRom(char* romPath, char* sramPath, bool loadDirect, bool loadGbaRom, char* gbaRom, char* gbaSram)
    {
        bool loaded = NDS::LoadROM(romPath, sramPath, loadDirect);
        if (!loaded)
            return 2;

        if (loadGbaRom)
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

        // TODO: create backup

        Savestate* savestate = new Savestate(path, false);
        if (savestate->Error)
        {
            delete savestate;
            success = false;

            // TODO: restore backup
        }

        NDS::DoSavestate(savestate);
        delete savestate;

        return success;
    }

    void cleanup()
    {
        NDS::DeInit();
        audioStream->requestStop();
        audioStream->close();
        audioStream = NULL;

        free(frameBuffer);
        frameBuffer = NULL;
    }
}

void Stop(bool internal)
{
}

bool LocalFileExists(const char* name)
{
    FILE* f = Platform::OpenFile(name, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}