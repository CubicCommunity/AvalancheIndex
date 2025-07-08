#include "../Avalanche.hpp"

#include <string>
#include <chrono>
#include <map>

#include <matjson.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace avalanche {
    Mod* AVAL_MOD = getMod(); // Get the mod instance

    int ACC_PUBLISHER = 31079132;
    std::string URL_MOD_ISSUES = AVAL_MOD->getMetadataRef().getIssues().value().url.value_or(URL_AVALANCHE); // URL to the mod's issues page on its GitHub repository

    matjson::Value fetchedBadges = nullptr; // Cached profile data
    matjson::Value fetchedLevels = nullptr; // Cached project data

    EventListener<web::WebTask> badgeListReq; // Web request listener for team profile data
    EventListener<web::WebTask> levelListReq; // Web request listener for team project data

    std::map<std::string, Profile::Badge> Profile::profileBadgeEnum{
        {"cubic-studios-badge"_spr, Profile::Badge::CUBIC},
        {"director-badge"_spr, Profile::Badge::DIRECTOR},
        {"team-manager-badge"_spr, Profile::Badge::MANAGER},
        {"team-member-badge"_spr, Profile::Badge::MEMBER},
        {"collaborator-badge"_spr, Profile::Badge::COLLABORATOR},
    };

    std::map<std::string, Project::Type> Project::projectTypeEnum{
        {"solo", Project::Type::SOLO},
        {"team", Project::Type::TEAM},
        {"collab", Project::Type::COLLAB},
        {"event", Project::Type::EVENT},
    };

    std::map<Profile::Badge, std::string> Handler::badgeStringID{
        {Profile::Badge::CUBIC, "cubic-studios-badge"_spr},
        {Profile::Badge::DIRECTOR, "director-badge"_spr},
        {Profile::Badge::MANAGER, "team-manager-badge"_spr},
        {Profile::Badge::MEMBER, "team-member-badge"_spr},
        {Profile::Badge::COLLABORATOR, "collaborator-badge"_spr},
    };

    std::map<std::string, std::string> Handler::badgeSpriteName{
        {Handler::badgeStringID[Profile::Badge::CUBIC], "cubic-studios.png"_spr},
        {Handler::badgeStringID[Profile::Badge::DIRECTOR], "director.png"_spr},
        {Handler::badgeStringID[Profile::Badge::MANAGER], "team-manager.png"_spr},
        {Handler::badgeStringID[Profile::Badge::MEMBER], "team-member.png"_spr},
        {Handler::badgeStringID[Profile::Badge::COLLABORATOR], "collaborator.png"_spr},
    };

    std::map<std::string, ccColor3B> Handler::badgeColor{
        {Handler::badgeStringID[Profile::Badge::CUBIC], AVAL_MOD->getSettingValue<ccColor3B>("com-cubic")},
        {Handler::badgeStringID[Profile::Badge::DIRECTOR], AVAL_MOD->getSettingValue<ccColor3B>("com-director")},
        {Handler::badgeStringID[Profile::Badge::MANAGER], AVAL_MOD->getSettingValue<ccColor3B>("com-manager")},
        {Handler::badgeStringID[Profile::Badge::MEMBER], AVAL_MOD->getSettingValue<ccColor3B>("com-member")},
        {Handler::badgeStringID[Profile::Badge::COLLABORATOR], AVAL_MOD->getSettingValue<ccColor3B>("com-collaborator")},
    };

    std::map<std::string, std::string> Handler::apiToString{
        {"cubic-studios", Handler::badgeStringID[Profile::Badge::CUBIC]},
        {"director", Handler::badgeStringID[Profile::Badge::DIRECTOR]},
        {"team-manager", Handler::badgeStringID[Profile::Badge::MANAGER]},
        {"team-member", Handler::badgeStringID[Profile::Badge::MEMBER]},
        {"collaborator", Handler::badgeStringID[Profile::Badge::COLLABORATOR]},
    };

    void Handler::scanAll() {
        badgeListReq.bind([](web::WebTask::Event* e) {
            if (web::WebResponse* avalReqRes = e->getValue()) {
                if (avalReqRes->ok()) {
                    if (fetchedBadges == nullptr) {
                        if (avalReqRes->json().isOk()) {
                            auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

                            for (auto& [key, value] : jsonRes) {
                                std::string cacheKey = fmt::format("cache-badge-p{}", (std::string)key);
                                AVAL_MOD->setSavedValue(cacheKey, value);
                            };

                            if (AVAL_MOD->getSettingValue<bool>("web-once")) fetchedBadges = jsonRes;
                        };
                    } else {
                        log::error("Already fetched remote data for badges");
                    };
                } else {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche badges", NotificationIcon::Error, 2.5f)->show();
                };
            } else if (web::WebProgress* p = e->getProgress()) {
                log::debug("badge id progress: {}", (float)p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::error("Badge web request failed");
                if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch badges", NotificationIcon::Error, 2.5f)->show();
            };
                          });

        auto badgeReq = web::WebRequest(); // send the web request
        badgeReq.userAgent("Avalanche Index mod for Geode");
        badgeReq.timeout(std::chrono::seconds(30));
        badgeListReq.setFilter(badgeReq.get(URL_API_BADGES));

        // fetch data for all projects
        levelListReq.bind([](web::WebTask::Event* e) {
            if (web::WebResponse* avalReqRes = e->getValue()) {
                if (avalReqRes->ok()) {
                    if (fetchedLevels == nullptr) {
                        if (avalReqRes->json().isOk()) {
                            auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

                            for (auto& [key, value] : jsonRes) {
                                std::string cacheKey = fmt::format("cache-level-p{}", (std::string)key);
                                AVAL_MOD->setSavedValue(cacheKey, value);
                            };

                            if (AVAL_MOD->getSettingValue<bool>("web-once")) fetchedLevels = jsonRes;
                        };
                    } else {
                        log::error("Already fetched remote data for levels");
                    };
                } else {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche levels", NotificationIcon::Error, 2.5f)->show();
                };
            } else if (web::WebProgress* p = e->getProgress()) {
                log::debug("level id progress: {}", (float)p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::error("Badge web request failed");
                if (AVAL_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch levels", NotificationIcon::Error, 2.5f)->show();
            };
                          });

        auto levelReq = web::WebRequest(); // send the web request
        levelReq.userAgent("Avalanche Index mod for Geode");
        levelReq.timeout(std::chrono::seconds(30));
        levelListReq.setFilter(levelReq.get(URL_API_LEVELS));
    };

    Profile Handler::GetProfile(int id) {
        if (id > 0) {
            std::string cacheKey = fmt::format("cache-badge-p{}", (int)id);

            matjson::Value cacheStd = AVAL_MOD->getSavedValue<matjson::Value>(cacheKey); // gets locally saved badge json

            auto lBadge = Profile::profileBadgeEnum.find(Handler::apiToString[cacheStd["badge"].asString().unwrapOr(und)]);

            auto c_name = cacheStd["name"].asString().unwrapOr(und);
            auto c_badge = (lBadge != Profile::profileBadgeEnum.end()) ? lBadge->second : Profile::Badge::NONE;

            Profile res(c_name, c_badge);
            return res;
        } else {
            log::error("Profile ID is invalid");

            Profile res("Name", Profile::Badge::NONE);
            return res;
        };
    };

    Project Handler::GetProject(int id) {
        if (id > 0) {
            std::string cacheKey = fmt::format("cache-level-p{}", (int)id);

            matjson::Value cacheStd = AVAL_MOD->getSavedValue<matjson::Value>(cacheKey); // gets locally saved level json

            auto lType = Project::projectTypeEnum.find(cacheStd["type"].asString().unwrapOr(und));

            auto c_name = cacheStd["name"].asString().unwrapOr(und);
            auto c_host = cacheStd["host"].asString().unwrapOr(und);
            auto c_showcase = cacheStd["showcase"].asString().unwrapOr(und);
            auto c_thumbnail = cacheStd["thumbnail"].asString().unwrapOr("");
            auto c_type = (lType != Project::projectTypeEnum.end()) ? lType->second : Project::Type::NONE;
            auto c_fame = cacheStd["fame"].asBool().unwrapOr(false);

            auto c_linked = cacheStd["project"];
            Project::LinkToMain ltm;

            if (c_linked.isObject()) {
                ltm.enabled = c_linked["enabled"].asBool().unwrapOr(false);
                ltm.level_id = c_linked["id"].asInt().unwrapOr(0);
            } else {
                ltm.enabled = false;
                ltm.level_id = 0;
            };

            Project res(c_name, c_host, c_showcase, c_thumbnail, c_type, c_fame, ltm);
            return res;
        } else {
            log::error("Project ID is invalid");

            Project res("Name", "Host", URL_AVALANCHE, "", Project::Type::NONE, false, Project::LinkToMain());
            return res;
        };
    };

    ccColor3B Handler::getCommentColor(Profile::Badge badge) {
        ccColor3B colorSetting = Handler::badgeColor[Handler::badgeStringID[badge]];
        return colorSetting;
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
        auto badge = Profile::profileBadgeEnum[nodeObject->getID()];

        Handler::getBadgeInfo(badge);
    };
};