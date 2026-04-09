#include "NDS.h"
#include "rcheevos.h"
#include "RetroAchievementsManager.h"
#include "types.h"

using namespace melonDS;


namespace MelonDSAndroid
{
namespace RetroAchievements
{

std::weak_ptr<MelonEventMessenger> RetroAchievementsManager::EventMessenger;
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

std::vector<RARuntimeAchievement> RetroAchievementsManager::GetRuntimeAchievements()
{
    std::vector<RARuntimeAchievement> achievements(loadedAchievements.size());
    int index = 0;
    for (const auto &item: loadedAchievements)
    {
        RARuntimeAchievement& runtimeAchievement = achievements[index++];
        runtimeAchievement.id = item.id;
        rc_runtime_get_achievement_measured(&rcheevosRuntime, item.id, &runtimeAchievement.value, &runtimeAchievement.target);
    }

    return achievements;
}

bool RetroAchievementsManager::DoSavestate(Savestate* savestate)
{
    std::unique_lock lock(runtimeLock);

    savestate->Section("RCHV");
    if (savestate->Saving)
    {
        u32 rcheevosStateSize = (u32) rc_runtime_progress_size(&rcheevosRuntime, nullptr);
        u8* rcheevosStateBuffer = new u8[rcheevosStateSize];
        int result = rc_runtime_serialize_progress_sized(rcheevosStateBuffer, rcheevosStateSize, &rcheevosRuntime, nullptr);
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
    auto eventMessenger = RetroAchievementsManager::EventMessenger.lock();
    if (!eventMessenger)
        return;

    switch (runtime_event->type)
    {
        case RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED:
            eventMessenger->onAchievementTriggered(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_PRIMED:
            eventMessenger->onAchievementPrimed(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_ACHIEVEMENT_UNPRIMED:
            eventMessenger->onAchievementUnprimed(runtime_event->id);
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
                eventMessenger->onAchievementProgressUpdated(runtime_event->id, value, target, buffer);
            }
            break;
        case RC_RUNTIME_EVENT_LBOARD_STARTED:
            eventMessenger->onLeaderboardAttemptStarted(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_LBOARD_CANCELED:
            eventMessenger->onLeaderboardAttemptCanceled(runtime_event->id);
            break;
        case RC_RUNTIME_EVENT_LBOARD_TRIGGERED:
        {
            std::string formattedValue = GetLeaderboardFormattedValue(runtime_event->id, runtime_event->value);
            eventMessenger->onLeaderboardAttemptCompleted(runtime_event->id, runtime_event->value, formattedValue);
            break;
        }
        case RC_RUNTIME_EVENT_LBOARD_UPDATED:
        {
            std::string formattedValue = GetLeaderboardFormattedValue(runtime_event->id, runtime_event->value);
            eventMessenger->onLeaderboardAttemptUpdated(runtime_event->id, formattedValue);
            break;
        }
    }
}

std::string RetroAchievementsManager::GetLeaderboardFormattedValue(int leaderboardId, int value)
{
    auto leaderboard = std::find_if(activeInstance->loadedLeaderboards.begin(), activeInstance->loadedLeaderboards.end(), [=](RALeaderboard l){ return l.id == leaderboardId; });
    char buffer[32];
    if (leaderboard != activeInstance->loadedLeaderboards.end())
        rc_runtime_format_lboard_value(buffer, sizeof(buffer), value, leaderboard->rcheevosFormat);
    else
        buffer[0] = '\0';

    return buffer;
}

unsigned PeekMemory(unsigned address, unsigned numBytes, void* ud)
{
    NDS* nds = (NDS*) ud;
    u8* memoryRegion;
    u32 memoryMask;

    // Obtain target region as defined in consoleinfo.c
    unsigned region = address & 0xFF000000;
    switch (region)
    {
        case 0x00000000U: // Main RAM
            memoryRegion = nds->MainRAM;
            memoryMask = nds->MainRAMMask;
            break;
        case 0x01000000U: // Data TCM
            memoryRegion = nds->ARM9.DTCM;
            memoryMask = DTCMPhysicalSize - 1;
            break;
        default:
            return 0;
    }

    switch (numBytes)
    {
        case 1:
        {
            u8 value = *(u8*) &memoryRegion[address & memoryMask];
            return value;
        }
        case 2:
        {
            u16 value  = *(u16*) &memoryRegion[address & memoryMask];
            return value;
        }
        case 4:
        {
            u32 value = *(u32*) &memoryRegion[address & memoryMask];
            return value;
        }
        default:
            return 0;
    }
}

}
}