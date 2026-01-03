#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <memory>
#include "renderer/Renderer.h"

namespace MelonDSAndroid
{

struct RenderSettings
{
};

struct SoftwareRenderSettings : public RenderSettings
{
    bool threadedRendering;
};

struct OpenGlRenderSettings : public RenderSettings
{
    bool betterPolygons;
    int scale;
    bool conservativeCoverageEnabled;
    float conservativeCoveragePx;
    float conservativeCoverageDepthBias;
    bool conservativeCoverageApplyRepeat;
    bool conservativeCoverageApplyClamp;
    bool debug3dClearMagenta;
};

struct ComputeRenderSettings : public RenderSettings
{
    int scale;
    bool highResCoordinates;
};

struct SdCardSettings
{
    bool enabled;
    char* imagePath;
    int imageSize;
    bool readOnly;
    bool folderSync;
    char* folderPath;
};

typedef struct
{
    char username[11];
    int language;
    int birthdayMonth;
    int birthdayDay;
    int favouriteColour;
    char message[27];
    bool randomizeMacAddress;
    char macAddress[18];
} FirmwareConfiguration;

typedef struct
{
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
    bool showBootScreen;
    bool useJit;
    int consoleType;
    bool soundEnabled;
    int volume;
    int audioInterpolation;
    int audioBitrate;
    int audioLatency;
    int micSource;
    int rewindEnabled;
    int rewindCaptureSpacingSeconds;
    int rewindLengthSeconds;
    FirmwareConfiguration firmwareConfiguration;
    std::unique_ptr<RenderSettings> renderSettings;
    SdCardSettings dsiSdCardSettings;
    SdCardSettings dldiSdCardSettings;
    Renderer renderer;
} EmulatorConfiguration;

}

#endif
