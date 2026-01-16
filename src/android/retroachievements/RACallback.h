#ifndef RACALLBACK_H
#define RACALLBACK_H

#include <string>

namespace MelonDSAndroid
{
namespace RetroAchievements
{

class RACallback
{
public:
    virtual void onAchievementPrimed(long achievementId) = 0;
    virtual void onAchievementTriggered(long achievementId) = 0;
    virtual void onAchievementUnprimed(long achievementId) = 0;
    virtual void onAchievementProgressUpdated(long achievementId, unsigned int current, unsigned int target, std::string progress) = 0;
    virtual void onLeaderboardAttemptStarted(long leaderboardId) = 0;
    virtual void onLeaderboardAttemptUpdated(long leaderboardId, std::string formattedValue) = 0;
    virtual void onLeaderboardAttemptCanceled(long leaderboardId) = 0;
    virtual void onLeaderboardAttemptCompleted(long leaderboardId, int value) = 0;
};

}
}

#endif //RACALLBACK_H
