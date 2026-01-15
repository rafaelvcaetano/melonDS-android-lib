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
};

}
}

#endif //RACALLBACK_H
