#ifndef RALEADERBOARD_H
#define RALEADERBOARD_H

#include <string>

namespace MelonDSAndroid
{
namespace RetroAchievements
{

    typedef struct RALeaderboard
    {
        long id;
        std::string memoryAddress;
        std::string format;
        int rcheevosFormat;
    } RALeaderboard;

}
}

#endif //RALEADERBOARD_H
