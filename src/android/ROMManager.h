/*
    Copyright 2016-2022 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include "types.h"
#include "SaveManager.h"
#include "AREngine.h"

#include <string>
#include <vector>

namespace ROMManager
{

enum SetupResult {
    SUCCESS = 0,
    BIOS9_MISSING,
    BIOS9_BAD,
    BIOS7_MISSING,
    BIOS7_BAD,
    FIRMWARE_MISSING,
    FIRMWARE_BAD,
    FIRMWARE_NOT_BOOTABLE,
    DSI_BIOS9_MISSING,
    DSI_BIOS9_BAD,
    DSI_BIOS7_MISSING,
    DSI_BIOS7_BAD,
    DSI_NAND_MISSING,
    DSI_NAND_BAD
};
}

#endif // ROMMANAGER_H
