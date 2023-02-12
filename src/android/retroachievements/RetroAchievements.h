#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H

#include <list>
#include "RAAchievement.h"
#include "RACallback.h"

namespace RetroAchievements
{

void Init(RACallback* callback);
bool LoadAchievements(std::list<RetroAchievements::RAAchievement> achievements);
void Reset();
void DeInit();
void FrameUpdate();
}

#endif //RETROACHIEVEMENTS_H
