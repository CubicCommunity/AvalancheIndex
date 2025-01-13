#include "util/Badges.hpp"

#include <string>
#include <vector>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>

#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/CCLayerColor.hpp>

#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/CommentCell.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/LevelCell.hpp>

using namespace geode::prelude;

// its modding time :3
auto getLoader = geode::Loader::get();
auto getThisMod = geode::getMod();

// array for ppl who've been checked
std::vector<std::string> checkedUsers;

// for fetching badges remotely
EventListener<web::WebTask> ogdBadgeRequest;

// creates badge button
void setNewBadge(std::string id, CCMenu *username_menu, float size, auto pointer)
{
	// checks the map for this value to see if its invalid
	bool idFailTest = Badges::badgeSpriteName[id].empty();

	if (idFailTest)
	{
		log::debug("Badge id '{}' is invalid.", id.c_str());
	}
	else
	{
		if (username_menu)
		{
			// prevent dupes
			if (auto alreadyBadge = username_menu->getChildByID(id))
			{
				alreadyBadge->removeMeAndCleanup();
			};

			// gets sprite filename
			auto newBadge = Badges::badgeSpriteName[id].c_str();

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Setting badge to {}...", newBadge);

			CCSprite *badgeSprite = CCSprite::create(newBadge);
			badgeSprite->setScale(size);

			CCMenuItemSpriteExtra *badge = CCMenuItemSpriteExtra::create(
				badgeSprite,
				pointer,
				menu_selector(Badges::onInfoBadge));
			badge->setID(id);
			badge->setZOrder(1);

			username_menu->addChild(badge);
			username_menu->updateLayout();

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Badge {} successfully set", newBadge);
		};
	};
};

// attempts fetch badge locally and remotely
void checkForBadge(CCMenu *username_menu, float size, auto pointer, int accID)
{
	// gets locally saved badge id
	std::string cacheStd = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)accID));
	auto badgeCache = cacheStd.c_str();

	if (getThisMod->getSettingValue<bool>("console"))
		log::debug("Checking if badge for user {} has been checked...", (int)accID);

	// look for this in the list of users already checked
	std::string search = std::to_string(accID);
	bool checked = false;

	for (const auto &element : checkedUsers)
	{
		if (element == search)
		{
			// set the bool as the mod setting's value
			checked = getThisMod->getSettingValue<bool>("web-once");
			break;
		};
	};

	if (checked)
	{
		if (getThisMod->getSettingValue<bool>("console"))
			log::error("Badge for user {} already been checked. Fetching badge from cache...", (int)accID);
	}
	else
	{
		if (getThisMod->getSettingValue<bool>("console"))
			log::warn("User not checked. Revising badge for user {} of ID '{}'...", (int)accID, badgeCache);

		// web request event
		ogdBadgeRequest.bind([pointer, username_menu, size, accID, cacheStd, search](web::WebTask::Event *e)
							 {
			if (web::WebResponse *ogdReqRes = e->getValue())
			{
				std::string ogdWebResUnwr = ogdReqRes->string().unwrapOr("404: Not Found");

				if (getThisMod->getSettingValue<bool>("console")) log::debug("Processing remotely-obtained string '{}'...", ogdWebResUnwr.c_str());

                if (ogdWebResUnwr.c_str() == cacheStd.c_str()) {
                    if (getThisMod->getSettingValue<bool>("console")) log::debug("Badge for user of ID {} up-to-date", (int)accID);
                } else {
					// check if badge map key is invalid
					bool failed = Badges::badgeSpriteName[ogdWebResUnwr].empty();
                    
					if (failed) {
						if (getThisMod->getSettingValue<bool>("console")) log::error("Badge of ID '{}' failed validation test", ogdWebResUnwr.c_str());
					} else {
                    	if (getThisMod->getSettingValue<bool>("console")) log::debug("Fetched badge {} remotely", ogdWebResUnwr.c_str());
					
                    	setNewBadge(ogdWebResUnwr, username_menu, size, pointer);
					};
					
					getThisMod->setSavedValue(fmt::format("cache-badge-u{}", (int)accID), ogdWebResUnwr);
                };

				// save the user id if its set to only check once per web
				if (getThisMod->getSettingValue<bool>("web-once"))
					checkedUsers.push_back(search);
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				if (getThisMod->getSettingValue<bool>("console")) log::debug("badge id progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				if (getThisMod->getSettingValue<bool>("console")) log::error("Badge web request failed");
				if (getThisMod->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch badge", NotificationIcon::Error, 2.5f)->show();
			}; });

		// send the web request
		auto ogdReq = web::WebRequest();
		ogdBadgeRequest.setFilter(ogdReq.get(fmt::format("https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/publicBadges/{}.txt", (int)accID)));
	};

	// checks the map with the cache as a key to see if its invalid
	bool isNotCached = Badges::badgeSpriteName[cacheStd].empty();

	if (isNotCached)
	{
		if (getThisMod->getSettingValue<bool>("console"))
			log::error("Badge id '{}' from cache is invalid", badgeCache);
	}
	else
	{
		if (getThisMod->getSettingValue<bool>("console"))
			log::debug("Fetched badge id '{}' from cache", badgeCache);
		setNewBadge(cacheStd, username_menu, size, pointer);
	};
};

class $modify(Profile, ProfilePage)
{
	// modified vanilla loadPageFromUserInfo function
	void loadPageFromUserInfo(GJUserScore *user)
	{
		ProfilePage::loadPageFromUserInfo(user);

		if (getThisMod->getSettingValue<bool>("show-profile"))
		{
			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu *username_menu = typeinfo_cast<CCMenu *>(mLayer->getChildByIDRecursive("username-menu"));

			checkForBadge(username_menu, 0.875f, this, user->m_accountID);

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Viewing profile of ID {}", user->m_accountID);
		};
	};
};

class $modify(Comment, CommentCell)
{
	// modified vanilla loadFromComment function
	void loadFromComment(GJComment *comment)
	{
		CommentCell::loadFromComment(comment);

		if (getThisMod->getSettingValue<bool>("show-comments"))
		{
			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu *username_menu = typeinfo_cast<CCMenu *>(mLayer->getChildByIDRecursive("username-menu"));

			checkForBadge(username_menu, 0.5f, this, comment->m_accountID);

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Viewing comment profile of ID {}", comment->m_accountID);
		};
	};
};

// TODO: Make this detect Avalanche team projects
class $modify(LevelInfo, LevelInfoLayer)
{
	bool init(GJGameLevel *level, bool challenge)
	{
		if (LevelInfoLayer::init(level, challenge))
		{
			auto bg = this->getChildByID("background");
			auto background = as<CCSprite *>(bg);

			background->setColor({66, 94, 255});

			return true;
		}
		else
		{
			return false;
		};
	};
};

// TODO: Make this work and detect Avalanche team projects
class $modify(Level, LevelCell)
{
	void loadFromLevel(GJGameLevel *p0)
	{
		LevelCell::loadFromLevel(p0);

		CCLayerColor *color = this->getChildByType<CCLayerColor *>(0);
		color->setColor({15, 22, 61});
		color->updateDisplayedColor({15, 22, 61});
	};
};