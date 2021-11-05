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

#ifndef REWINDMANAGER_H
#define REWINDMANAGER_H

#include <list>

#include "../types.h"

namespace RewindManager
{

struct RewindSaveState {
    u8* buffer;
    u32 bufferSize;
    u8* screenshot;
    u32 screenshotSize;
    int frame;
};

extern void SetRewindBufferSizes(u32 savestateSizeBytes, u32 screenshotSizeBytes);
extern bool ShouldCaptureState(int currentFrame);
extern RewindSaveState GetNextRewindSaveState(int currentFrame);
extern std::list<RewindSaveState> GetRewindWindow();
extern void OnRewindFromState(RewindSaveState state);
extern void TrimRewindWindowIfRequired();
extern void Reset();

}

#endif // REWINDMANAGER_H
