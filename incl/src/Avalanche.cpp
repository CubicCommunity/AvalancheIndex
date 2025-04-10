#include <string>
#include <chrono>
#include <map>

#include <matjson.hpp>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

#include "../Avalanche.hpp"

using namespace geode::prelude;

auto thisMod = geode::getMod();

namespace avalanche
{
    int ACC_PUBLISHER = 31079132;

    matjson::Value fetchedBadges = nullptr;
    matjson::Value fetchedLevels = nullptr;

    EventListener<web::WebTask> badgeListReq;
    EventListener<web::WebTask> levelListReq;

    std::map<std::string, avalanche::Profile::Badge> avalanche::Profile::profileBadgeEnum{
        {"cubic-studios-badge"_spr, avalanche::Profile::Badge::CUBIC},
        {"director-badge"_spr, avalanche::Profile::Badge::DIRECTOR},
        {"team-manager-badge"_spr, avalanche::Profile::Badge::MANAGER},
        {"team-member-badge"_spr, avalanche::Profile::Badge::MEMBER},
        {"collaborator-badge"_spr, avalanche::Profile::Badge::COLLABORATOR},
    };

    std::map<std::string, avalanche::Project::Type> avalanche::Project::projectTypeEnum{
        {"solo", avalanche::Project::Type::SOLO},
        {"team", avalanche::Project::Type::TEAM},
        {"collab", avalanche::Project::Type::COLLAB},
        {"event", avalanche::Project::Type::EVENT},
    };

    std::map<avalanche::Profile::Badge, std::string> avalanche::Handler::badgeStringID{
        {avalanche::Profile::Badge::CUBIC, "cubic-studios-badge"_spr},
        {avalanche::Profile::Badge::DIRECTOR, "director-badge"_spr},
        {avalanche::Profile::Badge::MANAGER, "team-manager-badge"_spr},
        {avalanche::Profile::Badge::MEMBER, "team-member-badge"_spr},
        {avalanche::Profile::Badge::COLLABORATOR, "collaborator-badge"_spr},
    };

    std::map<std::string, std::string> avalanche::Handler::badgeSpriteName{
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::CUBIC], "cubic-studios.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::DIRECTOR], "director.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MANAGER], "team-manager.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MEMBER], "team-member.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::COLLABORATOR], "collaborator.png"_spr},
    };

    std::map<std::string, ccColor3B> avalanche::Handler::badgeColor{
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::CUBIC], thisMod->getSettingValue<ccColor3B>("com-cubic")},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::DIRECTOR], thisMod->getSettingValue<ccColor3B>("com-director")},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MANAGER], thisMod->getSettingValue<ccColor3B>("com-manager")},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MEMBER], thisMod->getSettingValue<ccColor3B>("com-member")},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::COLLABORATOR], thisMod->getSettingValue<ccColor3B>("com-collaborator")},
    };

    std::map<std::string, std::string> avalanche::Handler::apiToString{
        {"cubic-studios", avalanche::Handler::badgeStringID[avalanche::Profile::Badge::CUBIC]},
        {"director", avalanche::Handler::badgeStringID[avalanche::Profile::Badge::DIRECTOR]},
        {"team-manager", avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MANAGER]},
        {"team-member", avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MEMBER]},
        {"collaborator", avalanche::Handler::badgeStringID[avalanche::Profile::Badge::COLLABORATOR]},
    };

    void avalanche::Handler::scanAll()
    {
        badgeListReq.bind([](web::WebTask::Event *e)
                          {
            if (web::WebResponse *avalReqRes = e->getValue())
            {
                if (avalReqRes->ok())
                {
                    if (fetchedBadges == nullptr)
                    {
                        if (avalReqRes->json().isOk())
                        {
                            auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

                            for (auto &[key, value] : jsonRes)
                            {
                                if (key.empty())
                                {
                                    log::error("Key for profile is empty or invalid");
                                }
                                else
                                {
                                    std::string cacheKey = "cache-badge-p" + key;
                                    thisMod->setSavedValue(cacheKey, value);
                                };
                            };

                            if (thisMod->getSettingValue<bool>("web-once"))
                                fetchedBadges = jsonRes;
                        };
                    }
                    else
                    {
                        log::error("Already fetched remote data for badges");
                    };
                }
                else
                {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (thisMod->getSettingValue<bool>("err-notifs"))
                        Notification::create("Unable to fetch main Avalanche badges", NotificationIcon::Error, 2.5f)->show();
                };
            }
            else if (web::WebProgress *p = e->getProgress())
            {
                log::debug("badge id progress: {}", (float)p->downloadProgress().value_or(0.f));
            }
            else if (e->isCancelled())
            {
                log::error("Badge web request failed");
                if (thisMod->getSettingValue<bool>("err-notifs"))
                    Notification::create("Unable to fetch badge", NotificationIcon::Error, 2.5f)->show();
            }; });

        auto badgeReq = web::WebRequest(); // send the web request
        badgeReq.userAgent("Avalanche Index mod for Geode");
        badgeReq.timeout(std::chrono::seconds(30));
        badgeListReq.setFilter(badgeReq.get(URL_BADGES));

        // fetch data for all projects
        levelListReq.bind([](web::WebTask::Event *e)
                          {
            if (web::WebResponse *avalReqRes = e->getValue())
            {
                if (avalReqRes->ok())
                {
                    if (fetchedLevels == nullptr)
                    {
                        if (avalReqRes->json().isOk())
                        {
                            auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

                            for (auto &[key, value] : jsonRes)
                            {
                                if (key.empty())
                                {
                                    log::error("Key for project is empty or invalid");
                                }
                                else
                                {
                                    std::string cacheKey = "cache-level-p" + key;
                                    thisMod->setSavedValue(cacheKey, value);
                                };
                            };

                            if (thisMod->getSettingValue<bool>("web-once"))
                                fetchedLevels = jsonRes;
                        };
                    }
                    else
                    {
                        log::error("Already fetched remote data for levels");
                    };
                }
                else
                {
                    log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
                    if (thisMod->getSettingValue<bool>("err-notifs"))
                        Notification::create("Unable to fetch main Avalanche levels", NotificationIcon::Error, 2.5f)->show();
                };
            }
            else if (web::WebProgress *p = e->getProgress())
            {
                log::debug("level id progress: {}", (float)p->downloadProgress().value_or(0.f));
            }
            else if (e->isCancelled())
            {
                log::error("Badge web request failed");
                if (thisMod->getSettingValue<bool>("err-notifs"))
                    Notification::create("Unable to fetch level", NotificationIcon::Error, 2.5f)->show();
            }; });

        auto levelReq = web::WebRequest(); // send the web request
        levelReq.userAgent("Avalanche Index mod for Geode");
        levelReq.timeout(std::chrono::seconds(30));
        levelListReq.setFilter(levelReq.get(URL_LEVELS));
    };

    avalanche::Profile avalanche::Handler::GetProfile(int id)
    {
        if (id > 0)
        {
            std::string cacheKey = std::string("cache-badge-p") + std::to_string(id);

            matjson::Value cacheStd = thisMod->getSavedValue<matjson::Value>(cacheKey); // gets locally saved badge json

            auto lBadge = avalanche::Profile::profileBadgeEnum.find(avalanche::Handler::apiToString[cacheStd["badge"].asString().unwrapOr(und)]);

            auto c_name = cacheStd["name"].asString().unwrapOr(und);
            auto c_badge = (lBadge != avalanche::Profile::profileBadgeEnum.end()) ? lBadge->second : avalanche::Profile::Badge::NONE;

            avalanche::Profile res(c_name, c_badge);
            return res;
        }
        else
        {
            log::error("Profile ID is invalid");

            avalanche::Profile res("Name", avalanche::Profile::Badge::NONE);
            return res;
        };
    };

    avalanche::Project avalanche::Handler::GetProject(int id)
    {
        if (id > 0)
        {
            std::string cacheKey = std::string("cache-level-p") + std::to_string(id);

            matjson::Value cacheStd = thisMod->getSavedValue<matjson::Value>(cacheKey); // gets locally saved level json

            auto lType = avalanche::Project::projectTypeEnum.find(cacheStd["type"].asString().unwrapOr(und));

            auto c_name = cacheStd["name"].asString().unwrapOr(und);
            auto c_host = cacheStd["host"].asString().unwrapOr(und);
            auto c_showcase = cacheStd["showcase"].asString().unwrapOr(und);
            auto c_type = (lType != avalanche::Project::projectTypeEnum.end()) ? lType->second : avalanche::Project::Type::NONE;
            auto c_fame = cacheStd["fame"].asBool().unwrapOr(false);

            avalanche::Project res(c_name, c_host, c_showcase, c_type, c_fame);
            return res;
        }
        else
        {
            log::error("Project ID is invalid");

            avalanche::Project res("Name", "Host", "https://avalanche.cubicstudios.xyz/", avalanche::Project::Type::NONE, false);
            return res;
        };
    };

    ccColor3B avalanche::Handler::getCommentColor(Profile::Badge badge)
    {
        ccColor3B colorSetting = avalanche::Handler::badgeColor[avalanche::Handler::badgeStringID[badge]];
        return colorSetting;
    };

    void avalanche::Handler::getBadgeInfo(avalanche::Profile::Badge badge)
    {
        switch (badge)
        {
        case avalanche::Profile::Badge::CUBIC:
            geode::createQuickPopup(
                "Cubic Studios",
                "This user is a <cy>staff member</c> of <cj>Cubic Studios</c>. They partake in the activities of a department of Cubic, and may supervise or join projects such as <cl>Avalanche</c>.",
                "OK", "Learn More",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://www.cubicstudios.xyz/");
                    };
                });
            break;

        case avalanche::Profile::Badge::DIRECTOR:
            geode::createQuickPopup(
                "Avalanche Director",
                "This user is the <co>director</c> of <cl>Avalanche</c>. They run the whole team.",
                "OK", "Learn More",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                    };
                });
            break;

        case avalanche::Profile::Badge::MANAGER:
            geode::createQuickPopup(
                "Avalanche Manager",
                "This user is a <cy>manager</c> of <cl>Avalanche</c>. They manage team projects and collaborations.",
                "OK", "Learn More",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                    };
                });
            break;

        case avalanche::Profile::Badge::MEMBER:
            geode::createQuickPopup(
                "Avalanche Team Member",
                "This user is a <cg>member</c> of <cl>Avalanche</c>. They partake in team projects and collaborations.",
                "OK", "Learn More",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                    };
                });
            break;

        case avalanche::Profile::Badge::COLLABORATOR:
            geode::createQuickPopup(
                "Team Collaborator",
                "This user is a <cg>collaborator</c> of <cl>Avalanche</c>. They've directly worked on the crew's or team's projects as an outsider.",
                "OK", "Learn More",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
                    };
                });
            break;

        default:
            geode::createQuickPopup(
                "Oops!",
                "This badge has <cr>no available information</c>. This is likely unintentional, please report it as an issue in the mod's repository.",
                "OK", "Report",
                [](auto, bool btn2)
                {
                    if (btn2)
                    {
                        web::openLinkInBrowser("https://www.github.com/CubicCommunity/AvalancheIndex/issues/");
                    };
                });
            break;
        };
    };

    // badge button event
    void avalanche::Handler::onInfoBadge(CCObject *sender)
    {
        // gets the node that triggered the function
        auto nodeObject = as<CCNode *>(sender);
        auto badge = avalanche::Profile::profileBadgeEnum[nodeObject->getID()];

        avalanche::Handler::getBadgeInfo(badge);
    };
};