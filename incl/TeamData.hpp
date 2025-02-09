#ifndef TEAMDATA_HPP
#define TEAMDATA_HPP

#include <string>
#include <map>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace TeamData
{
    enum class BadgeID
    {
        CUBIC,
        DIRECTOR,
        MANAGER,
        MEMBER,
        COLLABORATOR,
    };

    enum class Project
    {
        NONE,
        TEAM,
        SOLO,
    };

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
        static std::map<BadgeID, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;
        static std::map<std::string, Color> badgeColor;

        void onInfoBadge(CCObject *sender);
    };
};

#endif // TEAMDATA_HPP