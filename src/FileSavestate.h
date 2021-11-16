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

#ifndef FILESAVESTATE_H
#define FILESAVESTATE_H

#include "Savestate.h"

class FileSavestate : public Savestate
{
public:
    FileSavestate(const char* filename, bool save);
    ~FileSavestate() override;

    void Section(const char* magic) override;
    void Var8(u8* var) override;
    void Var16(u16* var) override;
    void Var32(u32* var) override;
    void Var64(u64* var) override;
    void Bool32(bool* var) override;
    void VarArray(void* data, u32 len) override;

private:
    FILE* file;
};


#endif // FILESAVESTATE_H
