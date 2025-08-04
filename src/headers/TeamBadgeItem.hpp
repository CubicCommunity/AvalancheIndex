#pragma once

#include <Avalanche.hpp>

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace avalanche;

class TeamBadgeItem : public CCMenuItemSpriteExtra {
protected:
    Profile m_avalProfile;

    void activate(CCObject*);

    bool init(Profile profile, float scale = 1.f);

public:
    static TeamBadgeItem* create(Profile profile, float scale = 1.f);
};