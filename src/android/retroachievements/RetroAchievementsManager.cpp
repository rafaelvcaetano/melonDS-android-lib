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
RetroAchievementsManager* RetroAchievementsManager::activeInstance = nullptr;
std::mutex RetroAchievementsManager::activeInstanceLock;

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
    std::unique_lock lock(runtimeLock);

    for (const auto &achievement : achievements) {
        int result = rc_runtime_activate_achievement(&rcheevosRuntime, achievement.id, achievement.memoryAddress.c_str(), nullptr, 0);
        if (result != RC_OK)
            return false;

        loadedAchievements.push_back(achievement);
    }

    return true;
}

bool RetroAchievementsManager::LoadLeaderboards(std::list<RALeaderboard> leaderboards)
{
    std::unique_lock lock(runtimeLock);

    for (auto &leaderboard : leaderboards) {
        int result = rc_runtime_activate_lboard(&rcheevosRuntime, leaderboard.id, leaderboard.memoryAddress.c_str(), nullptr, 0);
        if (result != RC_OK)
            return false;

        int rcheevosLeaderboardType = rc_parse_format(leaderboard.format.c_str());
        leaderboard.rcheevosFormat = rcheevosLeaderboardType;

        loadedLeaderboards.push_back(leaderboard);
    }

    return true;
}

void RetroAchievementsManager::UnloadEverything()
{
    std::unique_lock lock(runtimeLock);

    for (const auto &achievement : loadedAchievements) {
        rc_runtime_deactivate_achievement(&rcheevosRuntime, achievement.id);
    }
    for (const auto &leaderboard : loadedLeaderboards) {
        rc_runtime_deactivate_lboard(&rcheevosRuntime, leaderboard.id);
    }

    loadedAchievements.clear();
    loadedLeaderboards.clear();
}

void RetroAchievementsManager::SetupRichPresence(std::string richPresenceScript)
{
    std::unique_lock lock(runtimeLock);

    rc_runtime_activate_richpresence(&rcheevosRuntime, richPresenceScript.c_str(), nullptr, 0);
    isRichPresenceEnabled = true;
}

std::string RetroAchievementsManager::GetRichPresenceStatus()
{
    std::unique_lock lock(runtimeLock);

    if (!isRichPresenceEnabled)
        return "";

    char buffer[512];
    rc_runtime_get_richpresence(&rcheevosRuntime, buffer, 512, PeekMemory, nds, nullptr);

    return buffer;
}

bool RetroAchievementsManager::DoSavestate(Savestate* savestate)
{
    std::unique_lock lock(runtimeLock);

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
    std::unique_lock lock(runtimeLock);
    rc_runtime_reset(&rcheevosRuntime);
}

void RetroAchievementsManager::FrameUpdate()
{
    std::unique_lock lock(runtimeLock);
    std::unique_lock instanceLock(activeInstanceLock);

    activeInstance = this;
    rc_runtime_do_frame(&rcheevosRuntime, &CheevosEventHandler, &PeekMemory, nds, nullptr);
    activeInstance = nullptr;
}

void RetroAchievementsManager::CheevosEventHandler(const rc_runtime_event_t* runtime_event)
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
        case RC_RUNTIME_EVENT_ACHIEVEMENT_PROGRESS_UPDATED:
            unsigned int value;
            unsigned int target;

            rc_runtime_get_achievement_measured(&activeInstance->rcheevosRuntime, runtime_event->id, &value, &target);
            // Do not notify of achievements with no progress. Weird, but it happens
            if (value > 0)
            {
                char buffer[32];
                rc_runtime_format_achievement_measured(&activeInstance->rcheevosRuntime, runtime_event->id, buffer, sizeof(buffer));
                RetroAchievementsManager::AchievementsCallback->onAchievementProgressUpdated(runtime_event->id, value, target, buffer);
            }
            break;
        case RC_RUNTIME_EVENT_LBOARD_STARTED:
            RetroAchievementsManager::AchievementsCallback->onLeaderboardAttemptStarted(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_LBOARD_CANCELED:
            RetroAchievementsManager::AchievementsCallback->onLeaderboardAttemptCanceled(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_LBOARD_TRIGGERED:
            RetroAchievementsManager::AchievementsCallback->onLeaderboardAttemptCompleted(runtime_event->id, runtime_event->value);
            break;
        case RC_RUNTIME_EVENT_LBOARD_UPDATED:
            auto leaderboard = std::find_if(activeInstance->loadedLeaderboards.begin(), activeInstance->loadedLeaderboards.end(), [=](RALeaderboard l){ return l.id == runtime_event->id; });
            if (leaderboard != activeInstance->loadedLeaderboards.end())
            {
                char buffer[32];
                rc_runtime_format_lboard_value(buffer, sizeof(buffer), runtime_event->value, leaderboard->rcheevosFormat);
                RetroAchievementsManager::AchievementsCallback->onLeaderboardAttemptUpdated(runtime_event->id, buffer);
            }
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