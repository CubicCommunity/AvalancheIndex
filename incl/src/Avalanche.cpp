#include <string>
#include <chrono>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

#include "../Avalanche.hpp"

using namespace geode::prelude;

namespace avalanche
{
    int ACC_PUBLISHER = 31079132;

    matjson::Value fetchedBadges = nullptr;
    matjson::Value fetchedLevels = nullptr;

    EventListener<web::WebTask> badgeListReq;
    EventListener<web::WebTask> levelListReq;

    auto thisMod = geode::getMod();

    std::map<std::string, avalanche::Profile::Badge> avalanche::Profile::profileBadgeEnum{
        {"cubic-studios", avalanche::Profile::Badge::CUBIC},
        {"director", avalanche::Profile::Badge::DIRECTOR},
        {"team-manager", avalanche::Profile::Badge::MANAGER},
        {"team-member", avalanche::Profile::Badge::MEMBER},
        {"collaborator", avalanche::Profile::Badge::COLLABORATOR},
    };

    std::map<std::string, avalanche::Project::Type> avalanche::Project::projectTypeEnum{
        {"solo", avalanche::Project::Type::SOLO},
        {"team", avalanche::Project::Type::TEAM},
        {"collab", avalanche::Project::Type::COLLAB},
        {"entry", avalanche::Project::Type::ENTRY},
        {"event", avalanche::Project::Type::EVENT},
    };

    std::map<avalanche::Profile::Badge, std::string> avalanche::Handler::badgeStringID{
        {avalanche::Profile::Badge::CUBIC, "cubic-studios"},
        {avalanche::Profile::Badge::DIRECTOR, "director"},
        {avalanche::Profile::Badge::MANAGER, "team-manager"},
        {avalanche::Profile::Badge::MEMBER, "team-member"},
        {avalanche::Profile::Badge::COLLABORATOR, "collaborator"},
    };

    std::map<std::string, std::string> avalanche::Handler::badgeSpriteName{
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::CUBIC], "cubic-studios.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::DIRECTOR], "director.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MANAGER], "team-manager.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MEMBER], "team-member.png"_spr},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::COLLABORATOR], "collaborator.png"_spr},
    };

    std::map<std::string, avalanche::Color> avalanche::Handler::badgeColor{
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::CUBIC], {10, 247, 247}},
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::DIRECTOR], {150, 175, 255}}, // modified to be brighter than official branding
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MANAGER], {127, 148, 255}},  // modified to be brighter than official branding
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::MEMBER], {191, 201, 255}},   // modified to be brighter than official branding
        {avalanche::Handler::badgeStringID[avalanche::Profile::Badge::COLLABORATOR], {200, 200, 200}},
    };

    void avalanche::Handler::scanAll()
    {
        badgeListReq.bind([](web::WebTask::Event *e)
                          {
			if (web::WebResponse *avalReqRes = e->getValue())
			{
				if (avalReqRes->ok()) {
					auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

						for (auto& [key, value] : jsonRes) {
							thisMod->setSavedValue(fmt::format("cache-badge-p{}", key), value);
						};

						if (thisMod->getSettingValue<bool>("web-once")) fetchedBadges = jsonRes;

						thisMod->setSavedValue("passed-first-time-load", true);
				} else {
					log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
					if (thisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche badges", NotificationIcon::Error, 2.5f)->show();
				};
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				log::debug("badge id progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				log::error("Badge web request failed");
				if (thisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch badge", NotificationIcon::Error, 2.5f)->show();
			}; });

        // send the web request
        auto badgeReq = web::WebRequest();
        badgeReq.userAgent("Avalanche Index mod for Geode");
        badgeReq.timeout(std::chrono::seconds(30));
        badgeListReq.setFilter(badgeReq.get(URL_BADGES));

        // fetch data for all projects
        levelListReq.bind([](web::WebTask::Event *e)
                          {
			if (web::WebResponse *avalReqRes = e->getValue())
			{
				if (avalReqRes->ok()) {
					auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

						for (auto& [key, value] : jsonRes) {
							thisMod->setSavedValue(fmt::format("cache-level-p{}", key), value);
						};

						if (thisMod->getSettingValue<bool>("web-once")) fetchedBadges = jsonRes;

						thisMod->setSavedValue("passed-first-time-load", true);
				} else {
					log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
					if (thisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche levels", NotificationIcon::Error, 2.5f)->show();
				};
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				log::debug("level id progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				log::error("Badge web request failed");
				if (thisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch level", NotificationIcon::Error, 2.5f)->show();
			}; });

        // send the web request
        auto levelReq = web::WebRequest();
        levelReq.userAgent("Avalanche Index mod for Geode");
        levelReq.timeout(std::chrono::seconds(30));
        levelListReq.setFilter(levelReq.get(URL_LEVELS));
    };

    void avalanche::Handler::createBadge(avalanche::Profile::Badge id, CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer)
    {
        // checks the map for this value to see if its invalid
        bool idFailTest = avalanche::Handler::badgeSpriteName[id].empty();

        if (idFailTest)
        {
            log::debug("Badge is invalid.");
        }
        else
        {
            if (cell_menu != nullptr)
            {
                // prevent dupes
                if (auto alreadyBadge = cell_menu->getChildByID(id))
                {
                    alreadyBadge->removeMeAndCleanup();
                };

                // gets sprite filename
                auto newBadge = avalanche::Handler::badgeSpriteName[id].c_str();

                CCSprite *badgeSprite = CCSprite::create(newBadge);
                badgeSprite->setScale(size);

                CCMenuItemSpriteExtra *badge = CCMenuItemSpriteExtra::create(
                    badgeSprite,
                    pointer,
                    menu_selector(avalanche::Handler::onInfoBadge));
                badge->setID(id);
                badge->setZOrder(1);

                cell_menu->addChild(badge);
                cell_menu->updateLayout();
            };

            if (comment != nullptr)
            {
                avalanche::Color col = avalanche::Handler::badgeColor[id];

                comment->setColor({col.red, col.green, col.blue});
                comment->setOpacity(255);
            };
        };
    };

    avalanche::Profile avalanche::Handler::GetProfile(int id)
    {
        // gets locally saved badge json
        matjson::Value cacheStd = thisMod->getSavedValue<matjson::Value>(fmt::format("cache-badge-u{}", (int)id));

        auto c_name = cacheStd["name"].asString().unwrapOr(und);
        auto c_badge = avalanche::Profile::profileBadgeEnum[cacheStd["badge"].asString().unwrapOr(und)];

        avalanche::Profile res(c_name, c_badge);
        return res;
    };

    avalanche::Project avalanche::Handler::GetProject(int id)
    {
        // gets locally saved level json
        matjson::Value cacheStd = thisMod->getSavedValue<matjson::Value>(fmt::format("cache-level-p{}", (int)id));

        auto c_name = cacheStd["name"].asString().unwrapOr(und);
        auto c_showcase = cacheStd["showcase"].asString().unwrapOr(und);
        auto c_type = avalanche::Project::projectTypeEnum[cacheStd["type"].asString().unwrapOr(und)];
        auto c_fame = cacheStd["fame"].asBool().unwrapOr(false);

        avalanche::Project res(c_name, c_showcase, c_type, c_fame);
        return res;
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