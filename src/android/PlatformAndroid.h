#ifndef PLATFORMANDROID_H
#define PLATFORMANDROID_H

#include <string>
#include "Platform.h"

namespace melonDS
{
namespace Platform
{

melonDS::Platform::FileHandle* OpenInternalFile(const std::string path, FileMode mode);

}
}

#endif
