#ifndef AVALANCHE_HPP
#define AVALANCHE_HPP

#include <string>
#include <map>
#include <exception>

#include <cocos2d.h>
#include <matjson.hpp>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;
using namespace matjson;

namespace avalanche { // Avalanche Index mod namespace
    extern Mod* AVAL_MOD; // Get the mod instance

    extern Value fetchedBadges; // Cached profile data
    extern Value fetchedLevels; // Cached project data

    extern int ACC_PUBLISHER; // Account ID of Avalanche's level publisher account

    constexpr const char* URL_CUBIC = "https://www.cubicstudios.xyz/"; // URL to Cubic Studios's official website
    constexpr const char* URL_AVALANCHE = "https://avalanche.cubicstudios.xyz/"; // URL to Avalanche's official website

    constexpr const char* URL_API_BADGES = "https://api.cubicstudios.xyz/avalanche/v1/profiles"; // URL to remote JSON file containing all data on profiles
    constexpr const char* URL_API_LEVELS = "https://api.cubicstudios.xyz/avalanche/v1/projects"; // URL to remote JSON file containing all data on projects

    constexpr const char* und = "undefined";
    constexpr const char* err = "404: Not Found";

    // Get string equivalents for types
    namespace values {
        // String IDs for profiles
        namespace Profiles {
            // Profile type ID obtained from API
            namespace API {
                inline constexpr auto None = "none";
                inline constexpr auto Cubic = "cubic-studios";
                inline constexpr auto Director = "director";
                inline constexpr auto Manager = "team-manager";
                inline constexpr auto Member = "team-member";
                inline constexpr auto Collaborator = "collaborator";
            };

            // Node IDs for badges
            namespace Nodes {
                inline constexpr auto None = "";
                inline constexpr auto Cubic = "cubic-studios-badge"_spr;
                inline constexpr auto Director = "director-badge"_spr;
                inline constexpr auto Manager = "team-manager-badge"_spr;
                inline constexpr auto Member = "team-member-badge"_spr;
                inline constexpr auto Collaborator = "collaborator-badge"_spr;
            };

            // Sprite IDs for badges
            namespace Sprites {
                inline constexpr auto None = "";
                inline constexpr auto Cubic = "cubic-studios.png"_spr;
                inline constexpr auto Director = "director.png"_spr;
                inline constexpr auto Manager = "team-manager.png"_spr;
                inline constexpr auto Member = "team-member.png"_spr;
                inline constexpr auto Collaborator = "collaborator.png"_spr;
            };
        };

        // String IDs for profiles
        namespace Projects {
            // Profile type ID obtained from API
            namespace API {
                inline constexpr auto None = "none";
                inline constexpr auto Solo = "solo";
                inline constexpr auto Team = "team";
                inline constexpr auto Event = "event";
                inline constexpr auto Collab = "collaboration";
            };
        };
    };

    // Avalanche member profile class
    struct Profile {
        enum class Badge {
            NONE, // No badge
            CUBIC, // Staff of Cubic Studios
            DIRECTOR, // Leads the whole team
            MANAGER, // Helps keep things in check
            MEMBER, // Participates in projects
            COLLABORATOR, // Non-members of the team who also worked on a project
        };

        std::string name = "Name"; // Official pseudonym of the member
        Badge badge = Badge::NONE; // ID of the member's badge

        Profile( // Constructor
                std::string n,
                Badge b
        ) : name(n), badge(b) {};

        Profile() = default; // Default empty constructor
    };

    // Avalanche project level class
    struct Project {
        enum class Type {
            NONE, // Not a project
            SOLO, // A project that a member worked on by themself
            TEAM, // A project that members of the team worked on
            COLLAB, // A project that involves the work of Collaborators
            EVENT, // A project that resulted from a public or private event hosted by Avalanche
        };

        // Link to the main team project
        struct LinkToMain {
            bool enabled = false; // If the link is enabled
            int level_id = 104663075; // ID of the in-game level for the linked project

            LinkToMain( // Constructor
                       bool e,
                       int li
            ) : enabled(e), level_id(li) {};

            LinkToMain() = default; // Default empty constructor
        };

        std::string name = "Name"; // Official name of the level
        std::string host = "Host"; // Team member that hosted the level
        std::string showcase = URL_AVALANCHE; // Tiny YouTube video URL of the full showcase of the level
        std::string thumbnail = ""; // Imgur URL for a custom thumbnail for the level
        Type type = Type::NONE; // Type of project the level is featured as
        bool fame = false; // If the level will be highlighted on lists

        LinkToMain link_to_main = LinkToMain(); // Optional link to the main team project

        Project( // Constructor
                std::string n,
                std::string h,
                std::string su,
                std::string ct,
                Type t,
                bool f,
                LinkToMain ltm
        ) : name(n), host(h), showcase(su), thumbnail(ct), type(t), fame(f), link_to_main(ltm) {};

        Project() = default; // Default empty constructor
    };

    class Handler {
    public:
        // Get the Handler functions
        static Handler* get() {
            static Handler ptr;
            return &ptr;
        };

        class Badges {
        public:
            // Get the node ID from the badge type enum
            static const char* getBadgeID(Profile::Badge badge);

            // Get the badge type enum from the node ID
            static Profile::Badge fromBadgeID(const std::string& id);

            // Get the texture name from the badge type enum
            static const char* getSpriteName(Profile::Badge badge);

            // Get the color from the badge type enum
            static ccColor3B getBadgeColor(Profile::Badge badge);

            // Get the node id from the API code
            static constexpr const char* apiToBadgeID(const std::string& apiName);
        };

        class Levels {
        public:
            // Get the project type enum from the API code
            static Project::Type fromString(const std::string& str);

            // Get the API code from the project type enum
            static constexpr const char* toString(Project::Type type);
        };

        // Fetch all remote data on badges and levels, automatically checks "Fetch Data Once" setting
        void scanAll();

        // Get profile data on a player
        Profile GetProfile(int id);
        // Get project data on a level
        Project GetProject(int id);

        // Check if the profile belongs to a team member
        bool isTeamMember(Profile::Badge badge);

        // Get the comment text color for a certain badge type
        ccColor3B getCommentColor(Profile::Badge badge);

        void getBadgeInfo(Profile::Badge badge);
        void onInfoBadge(CCObject* sender);

        // Create badge and format comment for a player
        template <typename T>
        void createBadge(
            T pointer, // Parent class
            Profile profile, // Member's profile object
            CCMenu* cell_menu, // Username menu to add the badge to
            TextArea* cmntText = nullptr, // Comment text node
            CCLabelBMFont* cmntFont = nullptr, // Comment font node
            float size = 0.625f // Scale of the badge sprite
        ) {
            log::debug("Creating badge for {}...", profile.name);
            auto idString = Handler::Badges::getBadgeID(profile.badge); // gets the string equivalent

            if (idString) {
                // get the badge item
                auto badge = Handler::Badges::fromBadgeID(std::string(idString));

                if (cell_menu == nullptr) {
                    log::debug("No username menu provided");
                } else {
                    log::debug("Found username menu for {}...", profile.name);

                    try {
                        // prevent dupes
                        if (auto alreadyBadge = cell_menu->getChildByID(idString)) alreadyBadge->removeMeAndCleanup();

                        auto newBadge = Handler::Badges::getSpriteName(badge); // gets sprite filename

                        CCSprite* badgeBtnSprite = CCSprite::createWithSpriteFrameName(newBadge);
                        badgeBtnSprite->setScale(size);

                        CCMenuItemSpriteExtra* badgeBtn = CCMenuItemSpriteExtra::create(
                            badgeBtnSprite,
                            pointer,
                            menu_selector(Handler::onInfoBadge));
                        badgeBtn->setID(idString);
                        badgeBtn->setZOrder(1);

                        cell_menu->addChild(badgeBtn);
                        cell_menu->updateLayout();
                    } catch (std::exception& e) {
                        log::error("Failed to create badge for {}...", profile.name);
                    };

                    log::info("Finished creating badge for {}", profile.name);
                };

                if (cmntText == nullptr && cmntFont == nullptr) {
                    log::debug("No comment text node provided");
                } else {
                    log::debug("Found comment text node for {}...", profile.name);
                    auto col = Handler::Badges::getBadgeColor(badge);

                    if (cmntText) {
                        cmntText->colorAllCharactersTo(col);
                        cmntText->setOpacity(255);
                    } else if (cmntFont) {
                        cmntFont->setColor(col);
                        cmntFont->setOpacity(255);
                    } else {
                        log::error("No comment text node found");
                    };

                    log::info("Finished changing comment text color for {}", profile.name);
                };
            } else {
                log::error("Badge is invalid.");
            };
        };
    };
};

#endif // AVALANCHE_HPP