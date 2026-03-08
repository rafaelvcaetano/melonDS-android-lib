#ifndef EMULATORARGSBUILDER_H
#define EMULATORARGSBUILDER_H

#include "Args.h"
#include "Configuration.h"

namespace MelonDSAndroid
{

std::optional<std::unique_ptr<melonDS::NDSArgs>> BuildArgsFromConfiguration(const EmulatorConfiguration& configuration, int instanceId);

}

#endif
