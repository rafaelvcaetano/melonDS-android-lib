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

#include <cstdio>
#include <cstring>

#include "MemorySavestate.h"

/*
    Memory savestate format

    header:
    00 - magic MELN

    section header:
    00 - section magic
    04 - section length
    08 - reserved
    0C - reserved

    Implementation details
*/

MemorySavestate::MemorySavestate(u8* buffer, bool save) : Savestate()
{
    Buffer = buffer;
    BufferPos = 0;

    VersionMajor = SAVESTATE_MAJOR;
    VersionMinor = SAVESTATE_MINOR;

    Error = false;

    if (save)
    {
        Saving = true;
        if (!buffer)
        {
            Error = true;
            return;
        }

        BufferWrite(MAGIC, 4);
    }
    else
    {
        Saving = false;
        if (!buffer)
        {
            Error = true;
            return;
        }

        u32 buf = 0;

        BufferRead(&buf, 4);
        if (buf != ((u32*) MAGIC)[0])
        {
            printf("MemorySavestate: invalid magic %08X\n", buf);
            Error = true;
            return;
        }
    }

    CurSection = -1;
}

MemorySavestate::~MemorySavestate()
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        if (CurSection != 0xFFFFFFFF)
        {
            u32 pos = BufferPos;
            BufferSeek(CurSection + 4);

            u32 len = pos - CurSection;
            BufferWrite(&len, 4);

            BufferSeek(pos);
        }
    }
}

void MemorySavestate::Section(const char* magic)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        if (CurSection != 0xFFFFFFFF)
        {
            u32 pos = BufferPos;
            BufferSeek(CurSection + 4);

            u32 len = pos - CurSection;
            BufferWrite(&len, 4);

            BufferSeek(pos);
        }

        CurSection = BufferPos;

        BufferWrite(magic, 4);
        BufferSeek(BufferPos + 12);
    }
    else
    {
        BufferSeek(HEADER_SIZE);

        for (;;)
        {
            u32 buf = 0;

            BufferRead(&buf, 4);
            if (buf != ((u32*) magic)[0])
            {
                if (buf == 0)
                {
                    printf("savestate: section %s not found. blarg\n", magic);
                    return;
                }

                buf = 0;
                BufferRead(&buf, 4);
                BufferSeek(BufferPos + buf - 8);
                continue;
            }

            BufferSeek(BufferPos + 12);
            break;
        }
    }
}

void MemorySavestate::Var8(u8* var)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        BufferWrite(var, 1);
    }
    else
    {
        BufferRead(var, 1);
    }
}

void MemorySavestate::Var16(u16* var)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        BufferWrite(var, 2);
    }
    else
    {
        BufferRead(var, 2);
    }
}

void MemorySavestate::Var32(u32* var)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        BufferWrite(var, 4);
    }
    else
    {
        BufferRead(var, 4);
    }
}

void MemorySavestate::Var64(u64* var)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        BufferWrite(var, 8);
    }
    else
    {
        BufferRead(var, 8);
    }
}

void MemorySavestate::Bool32(bool* var)
{
    // for compability
    if (Saving)
    {
        u32 val = *var;
        Var32(&val);
    }
    else
    {
        u32 val;
        Var32(&val);
        *var = val != 0;
    }
}

void MemorySavestate::VarArray(void* data, u32 len)
{
    if (Error)
    {
        return;
    }

    if (Saving)
    {
        BufferWrite(data, len);
    }
    else
    {
        BufferRead(data, len);
    }
}

void MemorySavestate::BufferWrite(const void* data, u32 length)
{
    memcpy(&Buffer[BufferPos], data, length);
    BufferPos += length;
}

void MemorySavestate::BufferRead(void* into, u32 length)
{
    memcpy(into, &Buffer[BufferPos], length);
    BufferPos += length;
}

void MemorySavestate::BufferSeek(u32 position)
{
    BufferPos = position;
}
