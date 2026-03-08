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

#include "RewindManager.h"
#include <list>

namespace melonDS
{

const int FRAMES_PER_SECOND = 60;

RewindManager::RewindManager(bool enabled, int rewindLengthSeconds, int rewindCapturingIntervalSeconds, size_t rewindBufferSize, size_t screenshotBufferSize) :
    Enabled(enabled),
    RewindLengthSeconds(rewindLengthSeconds),
    RewindCapturingIntervalSeconds(rewindCapturingIntervalSeconds),
    SavestateBufferSize(rewindBufferSize),
    ScreenshotBufferSize(screenshotBufferSize)
{
}

RewindManager::~RewindManager()
{
    for (auto state : RewindWindow)
        DeleteRewindSaveState(state);
}

void RewindManager::UpdateRewindSettings(bool enabled, int rewindLengthSeconds, int rewindCapturingIntervalSeconds)
{
    RewindLengthSeconds = rewindLengthSeconds;
    RewindCapturingIntervalSeconds = rewindCapturingIntervalSeconds;

    if (Enabled)
    {
        if (!enabled)
            Reset();
        else
            TrimRewindWindowIfRequired();
    }

    Enabled = enabled;
}

bool RewindManager::ShouldCaptureState(int currentFrame)
{
    if (!Enabled || RewindWindowSize() == 0)
    {
        return false;
    }

    return currentFrame % (RewindCapturingIntervalSeconds * FRAMES_PER_SECOND) == 0;
}

RewindSaveState* RewindManager::GetNextRewindSaveState(int currentFrame)
{
    if (RewindWindow.size() < RewindWindowSize())
    {
        // Windows is not yet full. Create new savestate
        RewindSaveState newRewindSaveState = CreateRewindSaveState(currentFrame);
        RewindWindow.push_front(newRewindSaveState);
    }
    else
    {
        // Window is already full. Reuse the oldest savestate and move it to the front
        auto oldestState = RewindWindow.back();
        oldestState.frame = currentFrame;
        RewindWindow.pop_back();
        RewindWindow.push_front(oldestState);
    }

    return &*RewindWindow.begin();
}

std::list<RewindSaveState> RewindManager::GetRewindWindow()
{
    return RewindWindow;
}

void RewindManager::OnRewindFromState(RewindSaveState state)
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

RewindSaveState RewindManager::CreateRewindSaveState(int frame)
{
    return RewindSaveState {
        .buffer = new u8[SavestateBufferSize],
        .bufferSize = SavestateBufferSize,
        .bufferContentSize = 0,
        .screenshot = new u8[ScreenshotBufferSize],
        .screenshotSize = ScreenshotBufferSize,
        .frame = frame
    };
}

void RewindManager::TrimRewindWindowIfRequired()
{
    int windowSize = RewindWindowSize();
    while (RewindWindow.size() > windowSize)
    {
        auto lastState = RewindWindow.back();
        DeleteRewindSaveState(lastState);
        RewindWindow.pop_back();
    }
}

void RewindManager::Reset()
{
    for (auto state : RewindWindow)
    {
        DeleteRewindSaveState(state);
    }

    RewindWindow.clear();
}

void RewindManager::DeleteRewindSaveState(RewindSaveState state)
{
    delete[] state.buffer;
    delete[] state.screenshot;
}

}