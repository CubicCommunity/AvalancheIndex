#pragma once

#include <string>
#include <map>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Badges
{
public:
    enum class BadgeID
    {
        Cubic = 0,
        Director = 1,
        Manager = 2,
        Member = 3,
        Collaborator = 6
    };

    static std::map<BadgeID, std::string> badgeStringID;
    static std::map<std::string, std::string> badgeSpriteName;

    void onInfoBadge(CCObject *sender);
};