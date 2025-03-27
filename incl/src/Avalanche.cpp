#include "../Avalanche.hpp"

#include <string>
#include <chrono>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/utils/web.hpp>

using namespace avalanche;

using namespace geode::prelude;

auto getThisMod = geode::getMod();
auto getThisLoader = geode::Loader::get();

std::map<std::string, Profile::Badge> Profile::profileBadgeEnum{
    {"cubic-studios", Profile::Badge::CUBIC},
    {"director", Profile::Badge::DIRECTOR},
    {"team-manager", Profile::Badge::MANAGER},
    {"team-member", Profile::Badge::MEMBER},
    {"collaborator", Profile::Badge::COLLABORATOR}};

std::map<std::string, Project::Type> Project::projectTypeEnum{
    {"team", Project::Type::TEAM},
    {"collab", Project::Type::COLLAB},
    {"entry", Project::Type::ENTRY},
    {"event", Project::Type::EVENT},
};

void Handler::scanAll()
{
    badgeListReq.bind([](web::WebTask::Event *e)
                      {
			if (web::WebResponse *avalReqRes = e->getValue())
			{
				if (avalReqRes->ok()) {
					auto jsonRes = avalReqRes->json().unwrapOr(matjson::Value::object());

						for (auto& [key, value] : jsonRes) {
							getThisMod->setSavedValue(fmt::format("cache-badge-u{}", value), badge);
						};

						if (getThisMod->getSettingValue<bool>("web-once")) fetchedBadges = jsonRes;

						getThisMod->setSavedValue("passed-first-time-load", true);
				} else {
					log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
					if (getThisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche badges", NotificationIcon::Error, 2.5f)->show();
				};
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				log::debug("badge id progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				log::error("Badge web request failed");
				if (getThisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch badge", NotificationIcon::Error, 2.5f)->show();
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
							getThisMod->setSavedValue(fmt::format("cache-level-p{}", value), level);
						};

						if (getThisMod->getSettingValue<bool>("web-once")) fetchedBadges = jsonRes;

						getThisMod->setSavedValue("passed-first-time-load", true);
				} else {
					log::error("Badge web request failed: {}", avalReqRes->string().unwrapOr(und));
					if (getThisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch main Avalanche levels", NotificationIcon::Error, 2.5f)->show();
				};
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				log::debug("level id progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				log::error("Badge web request failed");
				if (getThisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch level", NotificationIcon::Error, 2.5f)->show();
			}; });

    // send the web request
    auto levelReq = web::WebRequest();
    levelReq.userAgent("Avalanche Index mod for Geode");
    levelReq.timeout(std::chrono::seconds(30));
    levelListReq.setFilter(levelReq.get(URL_LEVELS));
};

void Handler::createBadge(std::string id, CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer)
{
    // checks the map for this value to see if its invalid
    bool idFailTest = Badges::badgeSpriteName[id].empty();

    if (idFailTest)
    {
        log::debug("Badge id '{}' is invalid.", id.c_str());
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
            auto newBadge = Badges::badgeSpriteName[id].c_str();

            CCSprite *badgeSprite = CCSprite::create(newBadge);
            badgeSprite->setScale(size);

            CCMenuItemSpriteExtra *badge = CCMenuItemSpriteExtra::create(
                badgeSprite,
                pointer,
                menu_selector(Badges::onInfoBadge));
            badge->setID(id);
            badge->setZOrder(1);

            cell_menu->addChild(badge);
            cell_menu->updateLayout();
        };

        if (comment != nullptr)
        {
            Color col = Badges::badgeColor[id];

            comment->setColor({col.red, col.green, col.blue});
            comment->setOpacity(255);
        };
    };
};

Profile Handler::GetProfile(int id)
{
    // gets locally saved badge json
    matjson::Value cacheStd = getThisMod->getSavedValue<matjson::Value>(fmt::format("cache-badge-u{}", (int)id));

    auto c_name = cacheStd["name"].asString().unwrapOr(und);
    auto c_badge = cacheStd["badge"].asString().unwrapOr(und) || Profile::Badge::NONE;

    Profile badge = {c_name, c_badge};
    return badge;
};

Project Handler::GetProject(int id)
{
    // gets locally saved level json
    matjson::Value cacheStd = getThisMod->getSavedValue<matjson::Value>(fmt::format("cache-level-p{}", (int)id));

    auto c_name = cacheStd["name"].asString().unwrapOr(und);
    auto c_showcase = cacheStd["showcase"].asString().unwrapOr(und);
    auto c_type = Project::projectTypeEnum[cacheStd["type"].asString().unwrapOr(und)] || Project::Type::NONE;
    auto c_fame = cacheStd["fame"].asBool().unwrapOr(false);

    Project level = {c_name, c_showcase, c_type, c_fame};
    return level;
};