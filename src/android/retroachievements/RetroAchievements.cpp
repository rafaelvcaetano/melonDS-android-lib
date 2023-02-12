#include "RetroAchievements.h"
#include "types.h"
#include "NDS.h"
#include "rcheevos.h"

rc_runtime_t rcheevosRuntime;

void CheevosEventHandler(const rc_runtime_event_t* runtime_event);
unsigned PeekMemory(unsigned address, unsigned numBytes, void* ud);

RetroAchievements::RACallback* achievementsCallback;

void RetroAchievements::Init(RACallback* callback)
{
    achievementsCallback = callback;
    rc_runtime_init(&rcheevosRuntime);
}

bool RetroAchievements::LoadAchievements(std::list<RetroAchievements::RAAchievement> achievements)
{
    for (const auto &achievement : achievements) {
        int result = rc_runtime_activate_achievement(&rcheevosRuntime, achievement.id, achievement.memoryAddress.c_str(), nullptr, 0);
        if (result != RC_OK)
            return false;
    }

    return true;
}

void RetroAchievements::Reset()
{
    rc_runtime_reset(&rcheevosRuntime);
}

void RetroAchievements::DeInit()
{
    rc_runtime_destroy(&rcheevosRuntime);
}

void RetroAchievements::FrameUpdate()
{
    rc_runtime_do_frame(&rcheevosRuntime, &CheevosEventHandler, &PeekMemory, nullptr, nullptr);
}

void CheevosEventHandler(const rc_runtime_event_t* runtime_event)
{
    switch (runtime_event->type)
    {
        case RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED:
            achievementsCallback->onAchievementTriggered(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_PRIMED:
            achievementsCallback->onAchievementPrimed(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_UNPRIMED:
            achievementsCallback->onAchievementUnprimed(runtime_event->id);
            break;
    }
}

unsigned PeekMemory(unsigned address, unsigned numBytes, void* ud)
{
    switch (numBytes)
    {
        case 1:
        {
            u8 value = *(u8*) &NDS::MainRAM[address & NDS::MainRAMMask];
            return value;
        }
        case 2:
        {
            u16 value  = *(u16*) &NDS::MainRAM[address & NDS::MainRAMMask];
            return value;
        }
        case 4:
        {
            u32 value = *(u32*) &NDS::MainRAM[address & NDS::MainRAMMask];
            return value;
        }
        default:
            return 0;
    }
}