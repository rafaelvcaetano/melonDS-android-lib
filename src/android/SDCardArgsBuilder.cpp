#include "Configuration.h"
#include "FATStorage.h"
#include "SDCardArgsBuilder.h"

using namespace melonDS;

namespace MelonDSAndroid
{

constexpr u64 MB(u64 i)
{
    return i * 1024 * 1024;
}

constexpr u64 imgSizes[] = {0, MB(256), MB(512), MB(1024), MB(2048), MB(4096)};

std::optional<FATStorageArgs> getSDCardArgs(SdCardSettings settings) noexcept
{
    if (!settings.enabled)
        return std::nullopt;

    return FATStorageArgs {
        .Filename = settings.imagePath,
        .Size = imgSizes[settings.imageSize],
        .ReadOnly = settings.readOnly,
        .SourceDir = settings.folderSync ? std::make_optional(settings.folderPath) : std::nullopt
    };
}
}