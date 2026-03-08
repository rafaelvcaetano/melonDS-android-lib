#ifndef SDCARDARGSBUILDER_H
#define SDCARDARGSBUILDER_H

#include "Configuration.h"
#include "FATStorage.h"

namespace MelonDSAndroid
{

std::optional<melonDS::FATStorageArgs> getSDCardArgs(SdCardSettings settings) noexcept;

}

#endif