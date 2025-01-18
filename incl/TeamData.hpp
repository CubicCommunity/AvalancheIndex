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

    class Badges
    {
    public:
        static std::map<BadgeID, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;

        void onInfoBadge(CCObject *sender);
    };
};

#endif // TEAMDATA_HPP