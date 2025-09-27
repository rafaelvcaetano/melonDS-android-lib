/*
    Copyright 2016-2025 Arisotura

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
#include "types.h"

namespace melonDS
{

struct RewindSaveState {
    u8* buffer;
    u32 bufferSize;
    u32 bufferContentSize;
    u8* screenshot;
    u32 screenshotSize;
    int frame;
};

typedef struct {
    int currentFrame;
    std::list<melonDS::RewindSaveState> rewindStates;
} RewindWindow;

class RewindManager
{
public:
    RewindManager(bool enabled, int rewindLengthSeconds, int rewindCapturingIntervalSeconds, size_t rewindBufferSize, size_t screenshotBufferSize);
    ~RewindManager();
    void UpdateRewindSettings(bool enabled, int rewindLengthSeconds, int rewindCapturingIntervalSeconds);
    bool ShouldCaptureState(int currentFrame);
    RewindSaveState* GetNextRewindSaveState(int currentFrame);
    std::list<RewindSaveState> GetRewindWindow();
    void OnRewindFromState(RewindSaveState state);
    void Reset();

private:
    RewindSaveState CreateRewindSaveState(int frame);
    void TrimRewindWindowIfRequired();
    void DeleteRewindSaveState(RewindSaveState state);
    int RewindWindowSize() { return RewindLengthSeconds / RewindCapturingIntervalSeconds; };

private:
    bool Enabled;
    std::list<RewindSaveState> RewindWindow = std::list<RewindSaveState>();
    int RewindLengthSeconds;
    int RewindCapturingIntervalSeconds;
    u32 SavestateBufferSize;
    u32 ScreenshotBufferSize;
};

}

#endif // REWINDMANAGER_H
