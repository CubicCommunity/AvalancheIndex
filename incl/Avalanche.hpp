#pragma once

#include <string>
#include <map>

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

    extern matjson::Value fetchedBadges;
    extern matjson::Value fetchedLevels;

    extern EventListener<web::WebTask> badgeListReq;
    extern EventListener<web::WebTask> levelListReq;

    class Color
    {
    public:
        GLubyte red;
        GLubyte green;
        GLubyte blue;

        Color(int r = 0, int g = 0, int b = 0) : red(static_cast<GLubyte>(r)), green(static_cast<GLubyte>(g)), blue(static_cast<GLubyte>(b)) {};
    };

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
            static Handler instance; // Static instance of the class
            return instance;         // Return the singleton instance
        };

        void scanAll();

        static std::map<Profile::Badge, std::string> badgeStringID;
        static std::map<std::string, std::string> badgeSpriteName;
        static std::map<std::string, Color> badgeColor;

        void getBadgeInfo(Profile::Badge badge);
        void onInfoBadge(CCObject *sender);

        Profile GetProfile(int id);
        Project GetProject(int id);

        void createBadge(Profile::Badge id, CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer);
    };
};