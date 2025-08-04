#include "../TeamBadgeItem.hpp"

#include <fmt/core.h>

#include <Avalanche.hpp>

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace avalanche;

bool TeamBadgeItem::init(Profile profile, float scale) {
    m_avalProfile = profile;

    auto badgeSprite = CCSprite::createWithSpriteFrameName(Handler::Badges::getSpriteName(profile.badge));
    badgeSprite->setScale(scale);

    if (CCMenuItemSpriteExtra::init(badgeSprite, badgeSprite, this, menu_selector(TeamBadgeItem::activate))) {
        auto avalHandler = Handler::get();

        setID(Handler::Badges::getBadgeID(profile.badge));
        setSprite(badgeSprite);
        setZOrder(101);

        return true;
    } else {
        return false;
    };
};

void TeamBadgeItem::activate(CCObject*) {
    Handler::getBadgeInfo(m_avalProfile.badge, m_avalProfile.name);
};

TeamBadgeItem* TeamBadgeItem::create(Profile profile, float scale) {
    auto ret = new TeamBadgeItem();

    if (ret && ret->init(profile, scale)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};