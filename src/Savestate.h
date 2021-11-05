/*
    Copyright 2016-2021 Arisotura

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

#ifndef SAVESTATE_H
#define SAVESTATE_H

#include <stdio.h>
#include "types.h"

#define SAVESTATE_MAJOR 9
#define SAVESTATE_MINOR 0

class Savestate
{
public:
    bool Error;

    bool Saving;
    u32 VersionMajor;
    u32 VersionMinor;

    u32 CurSection;

    virtual ~Savestate() {};

    virtual void Section(const char* magic) = 0;

    virtual void Var8(u8* var) = 0;
    virtual void Var16(u16* var) = 0;
    virtual void Var32(u32* var) = 0;
    virtual void Var64(u64* var) = 0;

    virtual void Bool32(bool* var) = 0;

    virtual void VarArray(void* data, u32 len) = 0;

    bool IsAtleastVersion(u32 major, u32 minor)
    {
        if (VersionMajor > major) return true;
        if (VersionMajor == major && VersionMinor >= minor) return true;
        return false;
    }

protected:
    const char* MAGIC = "MELN";
};

#endif // SAVESTATE_H
