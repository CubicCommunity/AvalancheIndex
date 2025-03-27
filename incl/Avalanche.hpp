#pragma once

#include <string>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace avalanche
{
    int ACC_PUBLISHER = 31079132;

    static constexpr const char *URL_BADGES = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProfiles.json";
    static constexpr const char *URL_LEVELS = "https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/avalProjects.json";

    // undefined string
    static constexpr const char *und = "undefined";

    // error string
    static constexpr const char *err = "undefined";

    // saved json of badge data
    matjson::Value fetchedBadges = nullptr;

    // saved json of level data
    matjson::Value fetchedLevels = nullptr;

    EventListener<web::WebTask> badgeListReq;
    EventListener<web::WebTask> levelListReq;

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

        std::map<std::string, Badge> profileBadgeEnum;

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
            TEAM,   // A project that members of the team worked on
            COLLAB, // A project that involves the work of Collaborators
            ENTRY,  // A project that Avalanche made as an entry to another event
            EVENT,  // A project that resulted from a public or private event hosted by Avalanche
        };

        std::map<std::string, Type> projectTypeEnum;

        std::string name;
        std::string showcase_url;
        Type type;
        bool fame;

        Project(std::string n = "Name", std::string su = "https://avalanche.cubicstudios.xyz/", Type t = Type::NONE, bool f = false) : name(n), showcase_url(su), type(t), fame(f) {};
    };

    class Handler
    {
    public:
        void scanAll();

        static std::map<Profile::Badge, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;

        Profile GetProfile(int id);
        Project GetProject(int id);

        void createBadge(Profile::Badge id, CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer);
    };
};