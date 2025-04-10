#ifndef AVALANCHE_HPP
#define AVALANCHE_HPP

#include <string>
#include <map>

#include <cocos2d.h>
#include <matjson.hpp>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace avalanche // Avalanche namespace
{
    extern int ACC_PUBLISHER; // Account ID of Avalanche's level publisher account

    constexpr const char *URL_BADGES = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProfiles.json"; // URL to remote JSON file containing all data on profiles
    constexpr const char *URL_LEVELS = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProjects.json"; // URL to remote JSON file containing all data on projects

    constexpr const char *und = "undefined";
    constexpr const char *err = "404: Not Found";

    extern matjson::Value fetchedBadges; // JSON object of data on all badges pulled remotely
    extern matjson::Value fetchedLevels; // JSON object of data on all levels pulled remotely

    extern EventListener<web::WebTask> badgeListReq; // Web request listener for Profile data
    extern EventListener<web::WebTask> levelListReq; // Web request listener for Project data

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

        static std::map<std::string, Badge> profileBadgeEnum; // Convert a string to a Badge enum

        std::string name; // Official pseudonym of the member
        Badge badge;      // ID of the member's badge

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
            EVENT,  // A project that resulted from a public or private event hosted by Avalanche
        };

        static std::map<std::string, Type> projectTypeEnum; // Convert a string to a Type enum

        std::string name;         // Official name of the level
        std::string host;         // Team member that hosted the level
        std::string showcase_url; // Video URL of the full showcase of the level
        Type type;                // Type of project the level is featured as
        bool fame;                // If the level will be highlighted on lists

        Project(std::string n = "Name", std::string h = "Host", std::string su = "https://avalanche.cubicstudios.xyz/", Type t = Type::NONE, bool f = false) : name(n), host(h), showcase_url(su), type(t), fame(f) {};
    };

    class Handler
    {
    public:
        // Get the Handler functions
        static Handler &get()
        {
            static Handler instance;
            return instance;
        };

        // Fetch all remote data on badges and levels, automatically checks "Fetch Data Once" setting
        void scanAll();

        static std::map<Profile::Badge, std::string> badgeStringID; // Convert a Badge enum to a string ID
        static std::map<std::string, std::string> badgeSpriteName;  // Get the sprite from the string badge ID
        static std::map<std::string, ccColor3B> badgeColor;         // Get the color from the string badge ID
        static std::map<std::string, std::string> badgeToAPI;       // Convert badge node ID to API ID
        static std::map<std::string, std::string> apiToString;      // Convert API-provided string to string ID

        // Get profile data on a player
        Profile GetProfile(int id);
        // Get project data on a level
        Project GetProject(int id);

        // Get the comment text color for a certain badge type
        ccColor3B getCommentColor(Profile::Badge badge);

        void getBadgeInfo(Profile::Badge badge);
        void onInfoBadge(CCObject *sender);

        // Create the badge to appear next to the player's username
        template <typename T>
        void createBadge(Profile profile, CCMenu *cell_menu, TextArea *cmntText, CCLabelBMFont *cmntFont, float size, T pointer)
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
                if (cell_menu == nullptr)
                {
                    log::debug("No username menu provided");
                }
                else
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

                if (cmntText == nullptr && cmntFont == nullptr)
                {
                    log::debug("No comment text node provided");
                }
                else
                {
                    log::debug("Found comment text node for {}...", profile.name);

                    ccColor3B col = avalanche::Handler::badgeColor[idString];

                    if (cmntText)
                    {
                        cmntText->setColor(col);
                        cmntText->colorAllLabels(col);
                        cmntText->colorAllCharactersTo(col);
                        cmntText->setOpacity(255);
                    }
                    else if (cmntFont)
                    {
                        cmntFont->setColor(col);
                        cmntFont->setOpacity(255);
                    }
                    else
                    {
                        log::error("No comment text node found");
                    };

                    log::info("Finished changing comment text color for {}", profile.name);
                };
            };
        };
    };
};

#endif // AVALANCHE_HPP