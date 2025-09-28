#include "NDS.h"
#include "rcheevos.h"
#include "RetroAchievementsManager.h"
#include "types.h"

using namespace melonDS;


namespace MelonDSAndroid
{
namespace RetroAchievements
{

RACallback* RetroAchievementsManager::AchievementsCallback = nullptr;

void CheevosEventHandler(const rc_runtime_event_t* runtime_event);
unsigned PeekMemory(unsigned address, unsigned numBytes, void* ud);

RetroAchievementsManager::RetroAchievementsManager(melonDS::NDS* nds) : nds(nds)
{
    rc_runtime_init(&rcheevosRuntime);
    isRichPresenceEnabled = false;
}

RetroAchievementsManager::~RetroAchievementsManager()
{
    rc_runtime_destroy(&rcheevosRuntime);
}

bool RetroAchievementsManager::LoadAchievements(std::list<RAAchievement> achievements)
{
    for (const auto &achievement : achievements) {
        int result = rc_runtime_activate_achievement(&rcheevosRuntime, achievement.id, achievement.memoryAddress.c_str(), nullptr, 0);
        if (result != RC_OK)
            return false;
    }

    return true;
}

void RetroAchievementsManager::UnloadAchievements(std::list<RAAchievement> achievements)
{
    for (const auto &achievement : achievements) {
        rc_runtime_deactivate_achievement(&rcheevosRuntime, achievement.id);
    }
}

void RetroAchievementsManager::SetupRichPresence(std::string richPresenceScript)
{
    rc_runtime_activate_richpresence(&rcheevosRuntime, richPresenceScript.c_str(), nullptr, 0);
    isRichPresenceEnabled = true;
}

std::string RetroAchievementsManager::GetRichPresenceStatus()
{
    if (!isRichPresenceEnabled)
        return "";

    char buffer[512];
    rc_runtime_get_richpresence(&rcheevosRuntime, buffer, 512, PeekMemory, nds, nullptr);

    return buffer;
}

bool RetroAchievementsManager::DoSavestate(Savestate* savestate)
{
    savestate->Section("RCHV");
    if (savestate->Saving)
    {
        u32 rcheevosStateSize = (u32) rc_runtime_progress_size(&rcheevosRuntime, nullptr);
        u8* rcheevosStateBuffer = new u8[rcheevosStateSize];
        int result = rc_runtime_serialize_progress(rcheevosStateBuffer, &rcheevosRuntime, nullptr);
        if (result != RC_OK)
        {
            delete[] rcheevosStateBuffer;
            return false;
        }

        savestate->Var32(&rcheevosStateSize);
        savestate->VarArray(rcheevosStateBuffer, rcheevosStateSize);
        delete[] rcheevosStateBuffer;
    }
    else if (savestate->Error)
    {
        // RCHV section was not found
        return false;
    }
    else
    {
        u32 rcheevosStateSize;
        savestate->Var32(&rcheevosStateSize);
        u8* rcheevosStateBuffer = new u8[rcheevosStateSize];
        savestate->VarArray(rcheevosStateBuffer, rcheevosStateSize);

        int result = rc_runtime_deserialize_progress(&rcheevosRuntime, rcheevosStateBuffer, nullptr);
        delete[] rcheevosStateBuffer;

        if (result != RC_OK)
            return false;
    }

    return true;
}

void RetroAchievementsManager::Reset()
{
    rc_runtime_reset(&rcheevosRuntime);
}

void RetroAchievementsManager::FrameUpdate()
{
    rc_runtime_do_frame(&rcheevosRuntime, &CheevosEventHandler, &PeekMemory, nds, nullptr);
}

void CheevosEventHandler(const rc_runtime_event_t* runtime_event)
{
    if (!RetroAchievementsManager::AchievementsCallback)
        return;

    switch (runtime_event->type)
    {
        case RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED:
            RetroAchievementsManager::AchievementsCallback->onAchievementTriggered(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_PRIMED:
            RetroAchievementsManager::AchievementsCallback->onAchievementPrimed(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_UNPRIMED:
            RetroAchievementsManager::AchievementsCallback->onAchievementUnprimed(runtime_event->id);
            break;
    }
}

unsigned PeekMemory(unsigned address, unsigned numBytes, void* ud)
{
    NDS* nds = (NDS*) ud;
    u8* mainRam = nds->MainRAM;
    u32 mainRamMask = nds->MainRAMMask;

    switch (numBytes)
    {
        case 1:
        {
            u8 value = *(u8*) &mainRam[address & mainRamMask];
            return value;
        }
        case 2:
        {
            u16 value  = *(u16*) &mainRam[address & mainRamMask];
            return value;
        }
        case 4:
        {
            u32 value = *(u32*) &mainRam[address & mainRamMask];
            return value;
        }
        default:
            return 0;
    }
}

}
}