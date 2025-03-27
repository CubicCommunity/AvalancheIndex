#include <string>
#include <map>

#include "../../incl/Avalanche.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace avalanche;

namespace TeamData
{
    void getBadgeInfo(std::string badge_ID);

    class Color
    {
    public:
        GLubyte red;
        GLubyte green;
        GLubyte blue;

        Color(int r = 0, int g = 0, int b = 0) : red(static_cast<GLubyte>(r)), green(static_cast<GLubyte>(g)), blue(static_cast<GLubyte>(b)) {};
    };

    class Badges
    {
    public:
        static std::map<Profile::Badge, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;
        static std::map<std::string, Color> badgeColor;

        void onInfoBadge(CCObject *sender);
    };
};