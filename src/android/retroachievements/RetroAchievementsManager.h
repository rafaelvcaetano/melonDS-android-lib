#ifndef RETROACHIEVEMENTSMANAGER_H
#define RETROACHIEVEMENTSMANAGER_H

#include <list>
#include "NDS.h"
#include "RAAchievement.h"
#include "RACallback.h"
#include "rcheevos.h"
#include "Savestate.h"

namespace MelonDSAndroid
{
namespace RetroAchievements
{

class RetroAchievementsManager
{
public:
    RetroAchievementsManager(melonDS::NDS* nds);
    ~RetroAchievementsManager();
    bool LoadAchievements(std::list<RAAchievement> achievements);
    void UnloadAchievements(std::list<RAAchievement> achievements);
    void SetupRichPresence(std::string richPresenceScript);
    std::string GetRichPresenceStatus();
    bool DoSavestate(melonDS::Savestate* savestate);
    void Reset();
    void FrameUpdate();

    static void CheevosEventHandler(const rc_runtime_event_t* runtime_event);

    static RACallback* AchievementsCallback;

private:

    melonDS::NDS* nds;
    rc_runtime_t rcheevosRuntime;
    std::mutex runtimeLock;
    bool isRichPresenceEnabled;

    static RetroAchievementsManager* activeInstance;
    static std::mutex activeInstanceLock;
};

}
}

#endif //RETROACHIEVEMENTSMANAGER_H
