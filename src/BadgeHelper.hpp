#include <Debugger.hpp>

#include <Avalanche.hpp>

#include "./headers/TeamBadgeItem.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace avalanche;

class BadgeHelper {
public:
    // Get the BadgeHelper functions
    static BadgeHelper* get() {
        static BadgeHelper ptr;
        return &ptr;
    };

    /**
         * Create badge and format comment for a player
         *
         * @param profile Member's profile object
         * @param cell_menu Username menu to add the badge to
         * @param cmntText Comment text node
         * @param cmntFont Comment font node
         * @param size Scale of the badge sprite
         */
    void createBadge(
        Profile profile,
        CCMenu* cell_menu,
        TextArea* cmntText = nullptr,
        CCLabelBMFont* cmntFont = nullptr,
        float size = 0.625f
    ) {
        AVAL_LOG_DEBUG("Creating badge for {}...", profile.name);
        auto idString = Handler::Badges::getBadgeID(profile.badge); // gets the string equivalent

        if (idString) {
            // get the badge item
            auto badge = Handler::Badges::fromBadgeID(std::string(idString));

            if (cell_menu == nullptr) {
                AVAL_LOG_DEBUG("No username menu provided");
            } else {
                AVAL_LOG_DEBUG("Found username menu for {}...", profile.name);

                try {
                    // prevent dupes
                    if (auto alreadyBadge = cell_menu->getChildByID(idString)) alreadyBadge->removeMeAndCleanup();

                    auto newBadge = Handler::Badges::getSpriteName(badge); // gets sprite filename

                    CCSprite* badgeBtnSprite = CCSprite::createWithSpriteFrameName(newBadge);
                    badgeBtnSprite->setScale(size);

                    CCMenuItemSpriteExtra* badgeBtn = TeamBadgeItem::create(profile, size);

                    cell_menu->addChild(badgeBtn);
                    cell_menu->updateLayout();
                } catch (std::exception& e) {
                    AVAL_LOG_ERROR("Failed to create badge for {}...", profile.name);
                };

                AVAL_LOG_INFO("Finished creating badge for {}", profile.name);
            };

            if (cmntText == nullptr && cmntFont == nullptr) {
                AVAL_LOG_DEBUG("No comment text node provided");
            } else {
                AVAL_LOG_DEBUG("Found comment text node for {}...", profile.name);
                auto col = Handler::Badges::getBadgeColor(badge);

                if (cmntText) {
                    cmntText->colorAllCharactersTo(col);
                    cmntText->setOpacity(255);
                } else if (cmntFont) {
                    cmntFont->setColor(col);
                    cmntFont->setOpacity(255);
                } else {
                    AVAL_LOG_ERROR("No comment text node found");
                };

                AVAL_LOG_INFO("Finished changing comment text color for {}", profile.name);
            };
        } else {
            AVAL_LOG_ERROR("Badge is invalid.");
        };
    };
};