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

// projects account
int projectAccount = 31079132;

// array for ppl who've been checked
std::vector<std::string> checkedUsers;

// for fetching badges remotely
EventListener<web::WebTask> ogdBadgeRequest;

// creates badge button
void setUserBadge(std::string id, CCMenu *cell_menu, float size, auto pointer)
{
	// checks the map for this value to see if its invalid
	bool idFailTest = Badges::badgeSpriteName[id].empty();

	if (idFailTest)
	{
		log::debug("Badge id '{}' is invalid.", id.c_str());
	}
	else
	{
		if (cell_menu)
		{
			// prevent dupes
			if (auto alreadyBadge = cell_menu->getChildByID(id))
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

			cell_menu->addChild(badge);
			cell_menu->updateLayout();

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Badge {} successfully set", newBadge);
		};
	};
};

// attempts to fetch badge locally and remotely
void scanForUserBadge(CCMenu *cell_menu, float size, auto pointer, int itemID)
{
	// gets locally saved badge id
	std::string cacheStd = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)itemID));
	auto badgeCache = cacheStd.c_str();

	if (getThisMod->getSettingValue<bool>("console"))
		log::debug("Checking if badge for user {} has been checked...", (int)itemID);

	// look for this in the list of users already checked
	std::string search = std::to_string(itemID);
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
			log::error("Badge for user {} already been checked. Fetching badge from cache...", (int)itemID);
	}
	else
	{
		if (getThisMod->getSettingValue<bool>("console"))
			log::warn("User not checked. Revising badge for user {} of ID '{}'...", (int)itemID, badgeCache);

		// web request event
		ogdBadgeRequest.bind([pointer, cell_menu, size, itemID, cacheStd, search](web::WebTask::Event *e)
							 {
			if (web::WebResponse *ogdReqRes = e->getValue())
			{
				std::string ogdWebResUnwr = ogdReqRes->string().unwrapOr("404: Not Found");

				if (getThisMod->getSettingValue<bool>("console")) log::debug("Processing remotely-obtained string '{}'...", ogdWebResUnwr.c_str());

                if (ogdWebResUnwr.c_str() == cacheStd.c_str()) {
                    if (getThisMod->getSettingValue<bool>("console")) log::debug("Badge for user of ID {} up-to-date", (int)itemID);
                } else {
					// check if badge map key is invalid
					bool failed = Badges::badgeSpriteName[ogdWebResUnwr].empty();
                    
					if (failed) {
						if (getThisMod->getSettingValue<bool>("console")) log::error("Badge of ID '{}' failed validation test", ogdWebResUnwr.c_str());
					} else {
                    	if (getThisMod->getSettingValue<bool>("console")) log::debug("Fetched badge {} remotely", ogdWebResUnwr.c_str());
					
                    	setUserBadge(ogdWebResUnwr, cell_menu, size, pointer);
					};
					
					getThisMod->setSavedValue(fmt::format("cache-badge-u{}", (int)itemID), ogdWebResUnwr);
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
		ogdBadgeRequest.setFilter(ogdReq.get(fmt::format("https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/publicBadges/{}.txt", (int)itemID)));
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
		setUserBadge(cacheStd, cell_menu, size, pointer);
	};
};

class $modify(Profile, ProfilePage)
{
	// modified vanilla loadPageFromUserInfo function
	void loadPageFromUserInfo(GJUserScore *user)
	{
		ProfilePage::loadPageFromUserInfo(user);

		if (getThisMod->getSettingValue<bool>("badge-profile"))
		{
			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu *cell_menu = typeinfo_cast<CCMenu *>(mLayer->getChildByIDRecursive("username-menu"));

			scanForUserBadge(cell_menu, 0.875f, this, user->m_accountID);

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

		if (getThisMod->getSettingValue<bool>("badge-comments"))
		{
			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu *cell_menu = typeinfo_cast<CCMenu *>(mLayer->getChildByIDRecursive("username-menu"));

			scanForUserBadge(cell_menu, 0.5f, this, comment->m_accountID);

			if (getThisMod->getSettingValue<bool>("console"))
				log::debug("Viewing comment profile of ID {}", comment->m_accountID);
		};
	};
};

// highlights the level info layers
class $modify(LevelInfo, LevelInfoLayer)
{
	bool init(GJGameLevel *level, bool challenge)
	{
		if (LevelInfoLayer::init(level, challenge))
		{
			auto thisLevel = this->m_level;

			auto bg = this->getChildByID("background");
			auto background = as<CCSprite *>(bg);

			auto cacheSolo = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)thisLevel->m_accountID.value()));
			bool notSolo = Badges::badgeSpriteName[cacheSolo].empty() && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[Badges::BadgeID::Collaborator]] && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[Badges::BadgeID::Cubic]];

			bool displaySoloLayers = getThisMod->getSettingValue<bool>("solo-layers");
			bool displayTeamLayers = getThisMod->getSettingValue<bool>("team-layers");

			if (thisLevel->m_unlisted)
			{
				if (getThisMod->getSettingValue<bool>("console"))
					log::error("Level {} is unlisted", thisLevel->m_levelID.value());
			}
			else
			{
				if (displaySoloLayers)
				{
					if (notSolo)
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::error("Level {} is not an Avalanche team member solo", thisLevel->m_levelID.value());
					}
					else
					{
						if (thisLevel->m_stars.value() >= 1)
						{
							if (getThisMod->getSettingValue<bool>("console"))
								log::debug("Level {} is Avalanche team member solo", thisLevel->m_levelID.value());

							background->setColor({70, 77, 117});
						}
						else
						{
							if (getThisMod->getSettingValue<bool>("console"))
								log::error("Level {} not associated with Avalanche", thisLevel->m_levelID.value());
						};
					};
				};

				if (displayTeamLayers)
				{
					if (thisLevel->m_accountID.value() == projectAccount)
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::debug("Level {} is Avalanche team project", thisLevel->m_levelID.value());

						background->setColor({66, 94, 255});
					}
					else
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::error("Level {} not associated with Avalanche", thisLevel->m_levelID.value());
					};
				};
			};

			return true;
		}
		else
		{
			return false;
		};
	};
};

// highlights the level cells
class $modify(Level, LevelCell)
{
	void loadFromLevel(GJGameLevel *p0)
	{
		LevelCell::loadFromLevel(p0);

		auto thisLevel = this->m_level;

		auto cacheSolo = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)thisLevel->m_accountID.value()));
		bool notSolo = Badges::badgeSpriteName[cacheSolo].empty() && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[Badges::BadgeID::Collaborator]] && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[Badges::BadgeID::Cubic]];

		bool displaySoloCells = getThisMod->getSettingValue<bool>("solo-cells");
		bool displayTeamCells = getThisMod->getSettingValue<bool>("team-cells");

		auto color = this->getChildByType<CCLayerColor>(0);

		if (color)
		{
			if (thisLevel->m_unlisted)
			{
				if (getThisMod->getSettingValue<bool>("console"))
					log::error("Level {} is unlisted", thisLevel->m_levelID.value());
			}
			else
			{
				if (displaySoloCells)
				{
					if (notSolo)
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::error("Level {} is not an Avalanche team member solo", thisLevel->m_levelID.value());
					}
					else
					{
						if (displaySoloCells && thisLevel->m_stars.value() >= 1)
						{
							if (getThisMod->getSettingValue<bool>("console"))
								log::debug("Level {} is an Avalanche team member solo", thisLevel->m_levelID.value());

							auto newColor = CCLayerColor::create({70, 77, 117, 255});
							newColor->setScaledContentSize(color->getScaledContentSize());
							newColor->setAnchorPoint(color->getAnchorPoint());
							newColor->setPosition(color->getPosition());
							newColor->setZOrder(color->getZOrder() - 1);
							newColor->setID("solo_color"_spr);

							color->removeMeAndCleanup();
							this->addChild(newColor);
						}
						else
						{
							if (getThisMod->getSettingValue<bool>("console"))
								log::error("Level {} not associated with Avalanche", thisLevel->m_levelID.value());
						};
					};
				};

				if (displayTeamCells)
				{
					if (thisLevel->m_accountID.value() == projectAccount)
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::debug("Level {} is Avalanche team project", thisLevel->m_levelID.value());

						auto newColor = CCLayerColor::create({66, 94, 255, 255});
						newColor->setScaledContentSize(color->getScaledContentSize());
						newColor->setAnchorPoint(color->getAnchorPoint());
						newColor->setPosition(color->getPosition());
						newColor->setZOrder(color->getZOrder() - 1);
						newColor->setID("solo_color"_spr);

						color->removeMeAndCleanup();
						this->addChild(newColor);
					}
					else
					{
						if (getThisMod->getSettingValue<bool>("console"))
							log::error("Level {} not associated with Avalanche", thisLevel->m_levelID.value());
					};
				};
			};
		}
		else
		{
			log::error("Color not found!");
		};
	};
};