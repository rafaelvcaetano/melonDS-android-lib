#ifndef RAACHIEVEMENT_H
#define RAACHIEVEMENT_H

#include <string>

namespace MelonDSAndroid
{
namespace RetroAchievements
{

typedef struct RAAchievement
{
    long id;
    std::string memoryAddress;
} RAAchievement;

typedef struct RARuntimeAchievement
{
    long id;
    unsigned int value;
    unsigned int target;
} RARuntimeAchievement;

}
}

#endif //RAACHIEVEMENT_H
