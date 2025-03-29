#ifndef AVALANCHE_HPP
#define AVALANCHE_HPP

#include <string>
#include <map>

#include <cocos2d.h>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace avalanche
{
    extern int ACC_PUBLISHER;

    constexpr const char *URL_BADGES = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProfiles.json";
    constexpr const char *URL_LEVELS = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProjects.json";

    constexpr const char *und = "undefined";
    constexpr const char *err = "404: Not Found";

    extern matjson::Value fetchedBadges; // JSON object of data on all badges pulled remotely
    extern matjson::Value fetchedLevels; // JSON object of data on all levels pulled remotely

    extern EventListener<web::WebTask> badgeListReq;
    extern EventListener<web::WebTask> levelListReq;

    class Profile
    {
    public:
        enum class Badge
        {
            NONE,         // No badge
            CUBIC,        // Staff of Cubic Studios
            DIRECTOR,     // Leads the whole team
            MANAGER,      // Helps keep things in check
            MEMBER,       // Participates in projects
            COLLABORATOR, // Non-members of the team who also worked on a project
        };

        static std::map<std::string, Badge> profileBadgeEnum;

        std::string name;
        Badge badge;

        Profile(std::string n = "Name", Badge b = Badge::NONE) : name(n), badge(b) {};
    };

    class Project
    {
    public:
        enum class Type
        {
            NONE,   // Not a project
            SOLO,   // A project that a member worked on by themself
            TEAM,   // A project that members of the team worked on
            COLLAB, // A project that involves the work of Collaborators
            ENTRY,  // A project that Avalanche made as an entry to another event
            EVENT,  // A project that resulted from a public or private event hosted by Avalanche
        };

        static std::map<std::string, Type> projectTypeEnum;

        std::string name;
        std::string showcase_url;
        Type type;
        bool fame;

        Project(std::string n = "Name", std::string su = "https://avalanche.cubicstudios.xyz/", Type t = Type::NONE, bool f = false) : name(n), showcase_url(su), type(t), fame(f) {};
    };

    class Handler
    {
    public:
        static Handler &get()
        {
            static Handler instance;
            return instance;
        };

        void scanAll();

        static std::map<Profile::Badge, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;
        static std::map<std::string, ccColor3B> badgeColor;

        void getBadgeInfo(Profile::Badge badge);
        void onInfoBadge(CCObject *sender);

        Profile GetProfile(int id);
        Project GetProject(int id);

        // Create the badge to appear next to the player's username
        template <typename T>
        void createBadge(Profile profile, CCMenu *cell_menu, CCLabelBMFont *comment, float size, T pointer)
        {
            log::debug("Creating badge for {}...", profile.name);

            std::string idString = avalanche::Handler::badgeStringID[profile.badge]; // gets the string equivalent
            bool idFailTest = idString.empty();                                      // checks the map for this value to see if its invalid

            if (idFailTest)
            {
                log::error("Badge is invalid.");
            }
            else
            {
                if (cell_menu != nullptr)
                {
                    log::debug("Found username menu for {}...", profile.name);

                    try
                    {
                        // prevent dupes
                        if (auto alreadyBadge = cell_menu->getChildByID(idString))
                        {
                            alreadyBadge->removeMeAndCleanup();
                        };

                        auto newBadge = avalanche::Handler::badgeSpriteName[idString].c_str(); // gets sprite filename

                        CCSprite *badgeSprite = CCSprite::create(newBadge);
                        badgeSprite->setScale(size);

                        CCMenuItemSpriteExtra *badge = CCMenuItemSpriteExtra::create(
                            badgeSprite,
                            pointer,
                            menu_selector(avalanche::Handler::onInfoBadge));
                        badge->setID(idString);
                        badge->setZOrder(1);

                        cell_menu->addChild(badge);
                        cell_menu->updateLayout();
                    }
                    catch (...)
                    {
                        log::error("Failed to create badge for {}...", profile.name);
                    };

                    log::info("Finished creating badge for {}", profile.name);
                };

                if (comment != nullptr)
                {
                    log::debug("Found comment text for {}...", profile.name);

                    ccColor3B col = avalanche::Handler::badgeColor[idString];

                    comment->setColor({col.r, col.g, col.b});
                    comment->setOpacity(255);

                    log::info("Finished changing cpmment text color for {}", profile.name);
                };
            };
        };
    };
};

#endif // AVALANCHE_HPP