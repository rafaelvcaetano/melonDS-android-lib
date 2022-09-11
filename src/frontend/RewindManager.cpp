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

#include "RewindManager.h"
#include "Config.h"
#include <list>

namespace RewindManager
{

const int FRAMES_PER_SECOND = 60;

std::list<RewindSaveState> RewindWindow = std::list<RewindSaveState>();
u32 SavestateBufferSize;
u32 ScreenshotBufferSize;

int RewindWindowSize()
{
    return Config::RewindLengthSeconds / Config::RewindCaptureSpacingSeconds;
}

RewindSaveState CreateRewindSaveState(int frame)
{
    return RewindSaveState {
        .buffer = new u8[SavestateBufferSize],
        .bufferSize = SavestateBufferSize,
        .screenshot = new u8[ScreenshotBufferSize],
        .screenshotSize = ScreenshotBufferSize,
        .frame = frame
    };
}

void DeleteRewindSaveState(RewindSaveState state)
{
    delete state.buffer;
    delete state.screenshot;
}

void SetRewindBufferSizes(u32 savestateSizeBytes, u32 screenshotSizeBytes)
{
    SavestateBufferSize = savestateSizeBytes;
    ScreenshotBufferSize = screenshotSizeBytes;
}

bool ShouldCaptureState(int currentFrame)
{
    if (!Config::RewindEnabled || RewindWindowSize() == 0)
    {
        return false;
    }

    return currentFrame % (Config::RewindCaptureSpacingSeconds * FRAMES_PER_SECOND) == 0;
}

RewindSaveState GetNextRewindSaveState(int currentFrame)
{
    RewindSaveState nextRewindSaveState;
    if (RewindWindow.size() < RewindWindowSize())
    {
        // Windows is not yet full. Create new savestate
        RewindSaveState newRewindSaveState = CreateRewindSaveState(currentFrame);
        RewindWindow.push_front(newRewindSaveState);
        nextRewindSaveState = newRewindSaveState;
    }
    else
    {
        // Window is already full. Reuse the oldest savestate and move it to the front
        auto oldestState = RewindWindow.back();
        oldestState.frame = currentFrame;
        RewindWindow.pop_back();
        RewindWindow.push_front(oldestState);
        nextRewindSaveState = oldestState;
    }

    return nextRewindSaveState;
}

std::list<RewindSaveState> GetRewindWindow()
{
    return RewindWindow;
}

void OnRewindFromState(RewindSaveState state)
{
    auto iterator = RewindWindow.begin();
    while ((*iterator).frame > state.frame && iterator != RewindWindow.end())
    {
        RewindSaveState lastState = *iterator;
        DeleteRewindSaveState(lastState);
        iterator++;
        RewindWindow.pop_front();
    }
}

void TrimRewindWindowIfRequired()
{
    int windowSize = RewindWindowSize();
    while (RewindWindow.size() > windowSize)
    {
        auto lastState = RewindWindow.back();
        DeleteRewindSaveState(lastState);
        RewindWindow.pop_back();
    }
}

void Reset()
{
    for (auto state : RewindWindow)
    {
        DeleteRewindSaveState(state);
    }

    RewindWindow.clear();
}

}