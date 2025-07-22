#include "../Avalanche.hpp"

#include <algorithm>
#include <string>
#include <chrono>
#include <map>

#include <matjson.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace matjson;

namespace avalanche {
    Mod* AVAL_MOD = getMod(); // Get the mod instance

    Value fetchedBadges = Value();
    Value fetchedLevels = Value();

    int ACC_PUBLISHER = 31079132;
    std::string URL_MOD_ISSUES = AVAL_MOD->getMetadataRef().getIssues().value().url.value_or(URL_AVALANCHE); // URL to the mod's issues page on its GitHub repository

    EventListener<web::WebTask> badgeListReq; // Web request listener for team profile data
    EventListener<web::WebTask> levelListReq; // Web request listener for team project data

    const char* Handler::Badges::getBadgeID(Profile::Badge badge) {
        switch (badge) {
        case Profile::Badge::CUBIC: return values::Profiles::Nodes::Cubic;
        case Profile::Badge::DIRECTOR: return values::Profiles::Nodes::Director;
        case Profile::Badge::MANAGER: return values::Profiles::Nodes::Manager;
        case Profile::Badge::MEMBER: return values::Profiles::Nodes::Member;
        case Profile::Badge::COLLABORATOR: return values::Profiles::Nodes::Collaborator;

        default: return nullptr;
        };

        return nullptr;
    };

    Profile::Badge Handler::Badges::fromBadgeID(const std::string& id) {
        auto i = id.c_str();

        if (i == values::Profiles::Nodes::Cubic) return Profile::Badge::CUBIC;
        if (i == values::Profiles::Nodes::Director) return Profile::Badge::DIRECTOR;
        if (i == values::Profiles::Nodes::Manager) return Profile::Badge::MANAGER;
        if (i == values::Profiles::Nodes::Member) return Profile::Badge::MEMBER;
        if (i == values::Profiles::Nodes::Collaborator) return Profile::Badge::COLLABORATOR;

        return Profile::Badge::NONE;
    };

    const char* Handler::Badges::getSpriteName(Profile::Badge badge) {
        switch (badge) {
        case Profile::Badge::CUBIC: return values::Profiles::Sprites::Cubic;
        case Profile::Badge::DIRECTOR: return values::Profiles::Sprites::Director;
        case Profile::Badge::MANAGER: return values::Profiles::Sprites::Manager;
        case Profile::Badge::MEMBER: return values::Profiles::Sprites::Member;
        case Profile::Badge::COLLABORATOR: return values::Profiles::Sprites::Collaborator;

        default: return nullptr;
        };

        return nullptr;
    };

    ccColor3B Handler::Badges::getBadgeColor(Profile::Badge badge) {
        switch (badge) {
        case Profile::Badge::CUBIC: return AVAL_MOD->getSettingValue<ccColor3B>("com-cubic");
        case Profile::Badge::DIRECTOR: return AVAL_MOD->getSettingValue<ccColor3B>("com-director");
        case Profile::Badge::MANAGER: return AVAL_MOD->getSettingValue<ccColor3B>("com-manager");
        case Profile::Badge::MEMBER: return AVAL_MOD->getSettingValue<ccColor3B>("com-member");
        case Profile::Badge::COLLABORATOR: return AVAL_MOD->getSettingValue<ccColor3B>("com-collaborator");

        default: return { 255, 255, 255 };
        };

        return { 255, 255, 255 };
    };

    constexpr const char* Handler::Badges::apiToBadgeID(const std::string& apiName) {
        auto api = apiName.c_str();

        if (api == values::Profiles::API::Cubic) return getBadgeID(Profile::Badge::CUBIC);
        if (api == values::Profiles::API::Director) return getBadgeID(Profile::Badge::DIRECTOR);
        if (api == values::Profiles::API::Manager) return getBadgeID(Profile::Badge::MANAGER);
        if (api == values::Profiles::API::Member) return getBadgeID(Profile::Badge::MEMBER);
        if (api == values::Profiles::API::Collaborator) return getBadgeID(Profile::Badge::COLLABORATOR);

        return nullptr;
    };

    Project::Type Handler::Levels::fromString(const std::string& str) {
        auto s = str.c_str();

        if (s == values::Projects::API::Solo) return Project::Type::SOLO;
        if (s == values::Projects::API::Team) return Project::Type::TEAM;
        if (s == values::Projects::API::Collab) return Project::Type::COLLAB;
        if (s == values::Projects::API::Event) return Project::Type::EVENT;

        return Project::Type::NONE;
    };

    constexpr const char* Handler::Levels::toString(Project::Type type) {
        switch (type) {
        case Project::Type::SOLO: return values::Projects::API::Solo;
        case Project::Type::TEAM: return values::Projects::API::Team;
        case Project::Type::COLLAB: return values::Projects::API::Collab;
        case Project::Type::EVENT: return values::Projects::API::Event;

        default: return nullptr;
        };
    };

    void Handler::scanAll() {
        badgeListReq.bind([](web::WebTask::Event* e) {
            if (web::WebResponse* avalReqRes = e->getValue()) {
                if (avalReqRes->ok()) {
                    if (fetchedBadges.isNull()) {
                        if (avalReqRes->json().isOk()) {
                            auto jsonRes = avalReqRes->json().unwrapOr(Value::object());

                            if (jsonRes.contains("error")) { // if cubic's api returns an error object
                                auto errMsg = jsonRes["error"].asString().unwrapOr(und);
                                auto errCode = jsonRes["http_code"].asInt().unwrapOr(404);

                                log::error("[{}] Profile API request returned error: {}", errCode, errMsg);
                                jsonRes = Value::object();
                            } else {
                                for (auto& [key, value] : jsonRes) {
                                    auto cacheKey = fmt::format("cache-badge-p{}", (std::string)key);

                                    if (AVAL_MOD->getSettingValue<bool>("web-once")) fetchedBadges.set(key, value);
                                    AVAL_MOD->setSavedValue(cacheKey, value);
                                };
                            };
                        };
                    } else {
                        log::error("Already fetched remote data for badges");
                    };
                } else {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Failed to fetch main Avalanche badges", NotificationIcon::Error, 2.5f)->show();
                };
            } else if (web::WebProgress* p = e->getProgress()) {
                log::debug("badge id progress: {}", (float)p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::error("Badge web request failed");
                if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to reach API for Avalanche profiles", NotificationIcon::Error, 2.5f)->show();
            };
                          });

        auto badgeReq = web::WebRequest(); // send the web request
        badgeReq.userAgent("Avalanche Index mod for Geode");
        badgeReq.timeout(std::chrono::seconds(30));
        badgeListReq.setFilter(badgeReq.get(std::string(URL_API_BADGES)));

        // fetch data for all projects
        levelListReq.bind([](web::WebTask::Event* e) {
            if (web::WebResponse* avalReqRes = e->getValue()) {
                if (avalReqRes->ok()) {
                    if (fetchedLevels.isNull()) {
                        if (avalReqRes->json().isOk()) {
                            auto jsonRes = avalReqRes->json().unwrapOr(Value::object());

                            if (jsonRes.contains("error")) { // if cubic's api returns an error object
                                auto errMsg = jsonRes["error"].asString().unwrapOr(und);
                                auto errCode = jsonRes["http_code"].asInt().unwrapOr(404);

                                log::error("[{}] Project API request returned error: {}", errCode, errMsg);
                                jsonRes = Value::object();
                            } else {
                                for (auto& [key, value] : jsonRes) {
                                    auto cacheKey = fmt::format("cache-level-p{}", (std::string)key);

                                    if (AVAL_MOD->getSettingValue<bool>("web-once")) fetchedLevels.set(key, value);
                                    AVAL_MOD->setSavedValue(cacheKey, value);
                                };
                            };
                        };
                    } else {
                        log::error("Already fetched remote data for levels");
                    };
                } else {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Failed to fetch main Avalanche levels", NotificationIcon::Error, 2.5f)->show();
                };
            } else if (web::WebProgress* p = e->getProgress()) {
                log::debug("level id progress: {}", (float)p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::error("Badge web request failed");
                if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to reach API for Avalanche projects", NotificationIcon::Error, 2.5f)->show();
            };
                          });

        auto levelReq = web::WebRequest(); // send the web request
        levelReq.userAgent("Avalanche Index mod for Geode");
        levelReq.timeout(std::chrono::seconds(30));
        levelListReq.setFilter(levelReq.get(std::string(URL_API_LEVELS)));
    };

    Profile Handler::GetProfile(int id) {
        if (id > 0) {
            auto cacheKey = fmt::format("cache-badge-p{}", (int)id); // format the save key string
            Value cacheStd = AVAL_MOD->getSavedValue<Value>(cacheKey); // gets locally saved badge json

            if (AVAL_MOD->getSettingValue<bool>("web-once")) { // only fetch from cached web data if this setting is on
                log::debug("Fetching badge data from session web cache");

                if (fetchedBadges.isNull()) {
                    log::warn("Session web cache for badge data is unaccessible");
                } else if (fetchedBadges.isObject()) {
                    auto key = std::to_string(id);
                    fetchedBadges.contains(key) ? cacheStd = fetchedBadges.get(key).unwrapOr(cacheStd) : cacheStd = Value::object();
                };
            } else {
                log::warn("Fetching badge data directly from saved cache");
            };

            if (cacheStd.isNull()) {
                log::error("Player {} is no longer associated with Avalanche", (int)id);
                return Profile();
            } else {
                auto c_name = cacheStd["name"].asString().unwrapOr(und);
                auto c_badge = Handler::Badges::fromBadgeID(cacheStd["badge"].asString().unwrapOr(und));

                Profile res(c_name, c_badge);
                return res;
            };
        } else {
            log::error("Profile ID {} is invalid", (int)id);
            return Profile();
        };
    };

    Project Handler::GetProject(int id) {
        if (id > 0) {
            auto cacheKey = fmt::format("cache-level-p{}", (int)id); // format the save key string
            Value cacheStd = AVAL_MOD->getSavedValue<Value>(cacheKey); // gets locally saved level json

            if (AVAL_MOD->getSettingValue<bool>("web-once")) { // only fetch from cached web data if this setting is on
                log::debug("Fetching level data from session web cache");

                if (fetchedLevels.isNull()) {
                    log::warn("Session web cache for level data is unaccessible");
                } else if (fetchedLevels.isObject()) {
                    auto key = std::to_string(id);
                    fetchedLevels.contains(key) ? cacheStd = fetchedLevels.get(key).unwrapOr(cacheStd) : cacheStd = Value::object();
                };
            } else {
                log::warn("Fetching badge data directly from saved cache");
            };

            if (cacheStd.isNull()) {
                log::error("Level {} is no longer part of Avalanche", (int)id);
                return Project();
            } else {
                auto c_name = cacheStd["name"].asString().unwrapOr(und);
                auto c_host = cacheStd["host"].asString().unwrapOr(und);
                auto c_showcase = cacheStd["showcase"].asString().unwrapOr(und);
                auto c_thumbnail = cacheStd["thumbnail"].asString().unwrapOr(und);
                auto c_type = Handler::Levels::fromString(cacheStd["type"].asString().unwrapOr(und));
                auto c_fame = cacheStd["fame"].asBool().unwrapOr(false);

                auto c_linked = cacheStd["project"];
                Project::LinkToMain ltm;

                if (c_linked.isObject()) {
                    ltm = Project::LinkToMain(
                        c_linked["enabled"].asBool().unwrapOr(false),
                        c_linked["id"].asInt().unwrapOr(0)
                    );
                } else {
                    ltm = Project::LinkToMain();
                };

                Project res(c_name, c_host, c_showcase, c_thumbnail, c_type, c_fame, ltm);
                return res;
            };
        } else {
            log::error("Project ID is invalid");
            return Project();
        };
    };

    bool Handler::isTeamMember(Profile::Badge badge) {
        std::array<Profile::Badge, 3> badges = {
            Profile::Badge::DIRECTOR,
            Profile::Badge::MANAGER,
            Profile::Badge::MEMBER
        };

        return std::find(badges.begin(), badges.end(), badge) != badges.end();
    };

    ccColor3B Handler::getCommentColor(Profile::Badge badge) {
        return Handler::Badges::getBadgeColor(badge);
    };

    void Handler::getBadgeInfo(Profile::Badge badge) {
        auto title = "Oops!";
        auto description = "This badge has <cr>no available information</c>. This is likely unintentional, please report it as an issue in the mod's repository.";
        auto button = "Learn More";
        auto url = URL_AVALANCHE;

        switch (badge) {
        case Profile::Badge::CUBIC:
            title = "Cubic Studios";
            description = "This user is a <cy>staff member</c> of <cj>Cubic Studios</c>. They partake in the activities of a department of Cubic, and may supervise or join projects such as <cl>Avalanche</c>.";
            break;

        case Profile::Badge::DIRECTOR:
            title = "Avalanche Director";
            description = "This user is the <co>director</c> of <cl>Avalanche</c>. They run the whole team.";
            break;

        case Profile::Badge::MANAGER:
            title = "Avalanche Manager";
            description = "This user is a <cy>manager</c> of <cl>Avalanche</c>. They manage team projects and collaborations.";
            break;

        case Profile::Badge::MEMBER:
            title = "Avalanche Team Member";
            description = "This user is a <cg>member</c> of <cl>Avalanche</c>. They partake in team projects and collaborations.";
            break;

        case Profile::Badge::COLLABORATOR:
            title = "Team Collaborator";
            description = "This user is a <cg>collaborator</c> of <cl>Avalanche</c>. They've directly worked on the crew's or team's projects as an outsider.";
            break;

        default:
            button = "Report";
            url = URL_MOD_ISSUES.c_str();
            break;
        };

        createQuickPopup(
            title,
            description,
            "OK", button,
            [url](auto, bool btn2) {
                if (btn2) web::openLinkInBrowser(url);
            }, true);
    };

    // badge button event
    void Handler::onInfoBadge(CCObject* sender) {
        // gets the node that triggered the function
        auto nodeObject = as<CCNode*>(sender);
        auto badge = Handler::Badges::fromBadgeID(nodeObject->getID());

        Handler::getBadgeInfo(badge);
    };
};