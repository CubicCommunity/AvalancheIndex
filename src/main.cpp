#include "../incl/TeamData.hpp"
#include "../incl/ParticleHelper.hpp"
#include "../incl/AvalancheFeatured.hpp"

#include <string>
#include <vector>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>

#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/CommentCell.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/binding/GameLevelManager.hpp>

using namespace geode::prelude;
using namespace TeamData;

// its modding time :3
auto getThisMod = geode::getMod();
auto getThisLoader = geode::Loader::get();

// avalanche projects account
int projectAccount = 31079132;

// array for ppl who've been checked
std::vector<std::string> checkedUsers;

// for fetching badges remotely
EventListener<web::WebTask> avalBadgeRequest;

// if the server was checked for the new avalanche project :O
bool pinged = false;

// creates badge button
void setUserBadge(std::string id, CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer)
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

// attempts to fetch badge locally and remotely
void scanForUserBadge(CCMenu *cell_menu, CCLabelBMFont *comment, float size, auto pointer, int itemID)
{
	// gets locally saved badge id
	std::string cacheStd = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)itemID));
	auto badgeCache = cacheStd.c_str();

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
		log::error("Badge for user {} already been checked. Fetching badge from cache...", (int)itemID);
	}
	else
	{
		// web request event
		avalBadgeRequest.bind([pointer, cell_menu, comment, size, itemID, cacheStd, search](web::WebTask::Event *e)
							  {
			if (web::WebResponse *avalReqRes = e->getValue())
			{
				std::string avalWebResUnwr = avalReqRes->string().unwrapOr("404: Not Found");

                if (avalWebResUnwr.c_str() == cacheStd.c_str()) {
                    log::debug("Badge for user of ID {} up-to-date", (int)itemID);
                } else {
					// check if badge map key is invalid
					bool failed = Badges::badgeSpriteName[avalWebResUnwr].empty();
                    
					if (failed) {
						log::error("Badge of ID '{}' failed validation test", avalWebResUnwr.c_str());
					} else {
                    	if (cell_menu != nullptr && pointer != nullptr) {
							setUserBadge(avalWebResUnwr, cell_menu, comment, size, pointer);
						};
					};
					
					getThisMod->setSavedValue(fmt::format("cache-badge-u{}", (int)itemID), avalWebResUnwr);
                };

				// save the user id if its set to only check once per web
				if (getThisMod->getSettingValue<bool>("web-once")) checkedUsers.push_back(search);
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
		auto avalReq = web::WebRequest();
		avalBadgeRequest.setFilter(avalReq.get(fmt::format("https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/data/publicBadges/{}.txt", (int)itemID)));
	};

	// checks the map with the cache as a key to see if its invalid
	bool isNotCached = Badges::badgeSpriteName[cacheStd].empty();

	if (isNotCached)
	{
		log::error("Badge id '{}' from cache is invalid", badgeCache);
	}
	else
	{
		setUserBadge(cacheStd, cell_menu, comment, size, pointer);
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

			CCLabelBMFont *fakeText = nullptr;

			scanForUserBadge(cell_menu, fakeText, 0.875f, this, user->m_accountID);

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
			auto commentText = dynamic_cast<CCLabelBMFont *>(m_mainLayer->getChildByID("comment-text-label"));

			// checks if commenter published level
			log::debug("Checking comment on level of ID {}...", comment->m_levelID);

			if (getThisMod->getSettingValue<bool>("comments"))
			{
				if (comment->m_hasLevelID)
				{
					log::debug("Comment listed on account page with level linked");
				}
				else if (auto thisLevel = GameLevelManager::get()->getSavedLevel(comment->m_levelID))
				{
					if (comment->m_userID == thisLevel->m_userID.value())
					{
						log::info("Commenter {} is level publisher", comment->m_userName);

						commentText = nullptr;
					}
					else
					{
						log::debug("Commenter {} is not level publisher", comment->m_userName);
					};
				}
				else
				{
					log::debug("Comment not published under any level");
				};
			}
			else
			{
				commentText = nullptr;
			};

			scanForUserBadge(cell_menu, commentText, 0.5f, this, comment->m_accountID);

			log::debug("Viewing comment profile of ID {}", comment->m_accountID);
		};
	};
};

// attempts to fetch badge locally to verify ownership of the level
Project scanForLevelCreator(GJGameLevel *level)
{
	CCMenu *fakeMenu = nullptr;
	CCLabelBMFont *fakeText = nullptr;
	auto fakePointer = nullptr;
	scanForUserBadge(fakeMenu, fakeText, 0.5f, fakePointer, level->m_accountID);

	// get the member's badge data
	auto cacheSolo = getThisMod->getSavedValue<std::string>(fmt::format("cache-badge-u{}", (int)level->m_accountID.value()));
	bool notSolo = Badges::badgeSpriteName[cacheSolo].empty() && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[BadgeID::COLLABORATOR]] && Badges::badgeSpriteName[cacheSolo] != Badges::badgeSpriteName[Badges::badgeStringID[BadgeID::CUBIC]];
	bool notPublic = level->m_unlisted || level->m_friendsOnly;

	// must be public
	if (notPublic)
	{
		log::error("Level {} is unlisted", level->m_levelID.value());

		return Project::NONE;
	}
	else
	{
		log::debug("Level {} is publicly listed!", level->m_levelID.value());

		// checks if owned by publisher account
		if (level->m_accountID.value() == projectAccount)
		{
			log::debug("Level {} is Avalanche team project", level->m_levelID.value());

			return Project::TEAM;
		}
		else
		{
			// checks if level is published by a team member
			if (notSolo)
			{
				log::error("Level {} not associated with Avalanche", level->m_levelID.value());

				return Project::NONE;
			}
			else
			{
				// checks if level is rated
				if (level->m_stars.value() >= 1)
				{
					log::debug("Level {} is Avalanche team member solo", level->m_levelID.value());

					return Project::SOLO;
				}
				else
				{
					log::error("Level {} is unrated", level->m_levelID.value());

					return Project::NONE;
				};
			};
		};
	};
};

class $modify(LevelInfo, LevelInfoLayer)
{
	// modified vanilla init function
	bool init(GJGameLevel *level, bool challenge)
	{
		if (LevelInfoLayer::init(level, challenge))
		{
			bool displaySoloLayers = getThisMod->getSettingValue<bool>("solo-layers");
			bool displayTeamLayers = getThisMod->getSettingValue<bool>("team-layers");

			// get main bg color layer
			auto bg = this->getChildByID("background");
			auto background = as<CCSprite *>(bg);

			// whether or not display for classics only
			bool onlyClassic = getThisMod->getSettingValue<bool>("classic-only") && level->isPlatformer();

			auto levelType = scanForLevelCreator(level);

			if (levelType == Project::SOLO)
			{
				if (displaySoloLayers)
				{
					if (onlyClassic)
					{
						log::error("Solo level {} is platformer", level->m_levelID.value());
					}
					else
					{
						LevelInfo::setSoloDisplay(background);
					};
				};
			}
			else if (levelType == Project::TEAM)
			{
				if (displayTeamLayers)
				{
					if (level->isPlatformer())
					{
						log::error("Team level {} is platformer", level->m_levelID.value());
					}
					else if (level->m_unlisted)
					{
						log::error("Team level {} is unlisted", level->m_levelID.value());
					}
					else
					{
						LevelInfo::setTeamDisplay(background);
					};
				};

				// // discord rpc for viewing team levels (not working cuz of the rpc mod)
				// #if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)
				// #include <techstudent10.discord_rich_presence/include/CustomPresense.hpp>

				// 				using namespace gdrpc;

				// 				if (getThisLoader->isModLoaded("techstudent10.discord_rich_presence") && getThisMod->getSettingValue<bool>("discord"))
				// 				{
				// 					GDRPC::updateDiscordRP("Viewing Avalanche level");
				// 				};
				// #endif
			};

			return true;
		}
		else
		{
			return false;
		};
	};

	void setSoloDisplay(CCSprite *background)
	{
		background->setColor({70, 77, 117});
	};

	void setTeamDisplay(CCSprite *background)
	{
		auto bgSprite = CCSprite::createWithSpriteFrameName("game_bg_19_001.png");
		bgSprite->setColor({66, 94, 255});
		bgSprite->setAnchorPoint({0.5, 0.5});
		bgSprite->ignoreAnchorPointForPosition(false);
		bgSprite->setContentSize({this->getScaledContentWidth(), this->getScaledContentWidth()});
		bgSprite->setPosition({this->getScaledContentWidth() / 2, this->getScaledContentHeight() / 2});
		bgSprite->setZOrder(background->getZOrder());
		bgSprite->setID("team_background"_spr);

		auto bgThumbnail = CCSprite::create("background.png"_spr);
		bgThumbnail->setOpacity(75);
		bgThumbnail->setAnchorPoint({0, 0});
		bgThumbnail->ignoreAnchorPointForPosition(false);
		bgThumbnail->setPosition({0, 0});
		bgThumbnail->setZOrder(background->getZOrder() + 1);
		bgThumbnail->setID("team_thumbnail"_spr);

		float scaleFactor = bgSprite->getContentWidth() / bgThumbnail->getContentWidth();
		bgThumbnail->setScale(scaleFactor);

		background->setColor({66, 94, 255});
		background->setZOrder(-5);

		this->addChild(bgSprite);
		this->addChild(bgThumbnail);
	};
};

class $modify(Level, LevelCell)
{
	// modified vanilla loadFromLevel function
	void loadFromLevel(GJGameLevel *level)
	{
		LevelCell::loadFromLevel(level);

		bool displaySoloCells = getThisMod->getSettingValue<bool>("solo-cells");
		bool displayTeamCells = getThisMod->getSettingValue<bool>("team-cells");

		// get main bg color layer
		auto color = this->getChildByType<CCLayerColor>(0);

		// whether or not display for classics only
		bool onlyClassic = getThisMod->getSettingValue<bool>("classic-only") && level->isPlatformer();

		if (color)
		{
			auto levelType = scanForLevelCreator(level);

			if (levelType == Project::SOLO)
			{
				if (displaySoloCells)
				{
					if (onlyClassic)
					{
						log::error("Solo level {} is platformer", level->m_levelID.value());
					}
					else
					{
						Level::setSoloDisplay(color);
					};
				};
			}
			else if (levelType == Project::TEAM)
			{
				if (displayTeamCells)
				{
					if (level->isPlatformer())
					{
						log::error("Team level {} is platformer", level->m_levelID.value());
					}
					else if (level->m_unlisted)
					{
						log::error("Team level {} is unlisted", level->m_levelID.value());
					}
					else
					{
						Level::setTeamDisplay(color);
					};
				};
			};
		}
		else
		{
			log::error("Color not found!");
		};
	};

	void setSoloDisplay(CCLayerColor *color)
	{
		auto newColor = CCLayerColor::create({70, 77, 117, 255});
		newColor->setScaledContentSize(color->getScaledContentSize());
		newColor->setAnchorPoint(color->getAnchorPoint());
		newColor->setPosition(color->getPosition());
		newColor->setZOrder(color->getZOrder() - 2);
		newColor->setScale(color->getScale());
		newColor->setID("solo_color"_spr);

		color->removeMeAndCleanup();
		this->addChild(newColor);
	};

	void setTeamDisplay(CCLayerColor *color)
	{
		auto newColor = CCLayerColor::create({66, 94, 255, 255});
		newColor->setScaledContentSize(color->getScaledContentSize());
		newColor->setAnchorPoint(color->getAnchorPoint());
		newColor->setPosition(color->getPosition());
		newColor->setZOrder(color->getZOrder() - 2);
		newColor->setScale(color->getScale());
		newColor->setID("team_color"_spr);

		color->removeMeAndCleanup();
		this->addChild(newColor);
	};
};

class $modify(Menu, MenuLayer)
{
	struct Fields
	{
		EventListener<web::WebTask> avalWebListener;

		CCSprite *avalBtnGlow;
		CCSprite *avalBtnMark;
	};

	bool init()
	{
		if (MenuLayer::init())
		{
			auto winSizeX = this->getScaledContentWidth();
			auto winSizeY = this->getScaledContentHeight();

			bool showAvalButton = getThisMod->getSettingValue<bool>("show-aval-featured");

			if (showAvalButton)
			{
				// avalanche menu
				auto avalMenu = CCMenu::create();
				avalMenu->ignoreAnchorPointForPosition(false);
				avalMenu->setPosition(winSizeX / 2, (winSizeY / 2) - 70.f);
				avalMenu->setScaledContentSize(CCSize(winSizeX - 75.f, 50.f));

				this->addChild(avalMenu);

				auto avalBtnSprite = CCSprite::create("logo_circle_aval.png"_spr);
				avalBtnSprite->ignoreAnchorPointForPosition(false);
				avalBtnSprite->setScale(0.0875f);

				auto avalBtn = CCMenuItemSpriteExtra::create(
					avalBtnSprite,
					this,
					menu_selector(Menu::onAvalFeaturedButton));
				avalBtn->setPosition(avalMenu->getScaledContentWidth() / 2, avalMenu->getScaledContentHeight() / 2);
				avalBtn->setID("avalanche-featured-button"_spr);
				avalBtn->ignoreAnchorPointForPosition(false);

				avalMenu->addChild(avalBtn);

				// featured ring
				m_fields->avalBtnGlow = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
				m_fields->avalBtnGlow->setID("featuredRing"_spr);
				m_fields->avalBtnGlow->setScale(1.125f);
				m_fields->avalBtnGlow->ignoreAnchorPointForPosition(false);
				m_fields->avalBtnGlow->setPosition({avalBtnSprite->getScaledContentWidth() / 2, (avalBtnSprite->getScaledContentHeight() / 2) * 0.65f});
				m_fields->avalBtnGlow->setAnchorPoint({0.5f, 0.5f});
				m_fields->avalBtnGlow->setZOrder(-1);
				m_fields->avalBtnGlow->setVisible(false);

				avalBtn->addChild(m_fields->avalBtnGlow);

				m_fields->avalBtnMark = CCSprite::createWithSpriteFrameName("exMark_001.png");
				m_fields->avalBtnMark->setID("notifMark"_spr);
				m_fields->avalBtnMark->setScale(0.5f);
				m_fields->avalBtnMark->ignoreAnchorPointForPosition(false);
				m_fields->avalBtnMark->setPosition({avalBtn->getScaledContentWidth(), avalBtn->getScaledContentHeight()});
				m_fields->avalBtnMark->setAnchorPoint({0.5f, 0.5f});
				m_fields->avalBtnMark->setVisible(false);

				avalBtn->addChild(m_fields->avalBtnMark);

				// add particles on top of the featured button
				if (CCParticleSystem *avalBtnParticles = ParticleHelper::createAvalFeaturedParticles(100.0f))
				{
					avalBtnParticles->setPosition(avalBtn->getPosition());
					avalBtnParticles->setAnchorPoint({0.5f, 0.5f});
					avalBtnParticles->setScale(1.0f);
					avalBtnParticles->setZOrder(-2);
					avalBtnParticles->setStartColor({6, 2, 32, 255});
					avalBtnParticles->setEndColor({33, 33, 33, 100});
					avalBtnParticles->setEmissionRate(20.f);
					avalBtnParticles->setRotatePerSecond(22.5f);
					avalBtnParticles->setStartSize(5.f);
					avalBtnParticles->setEndSize(1.25f);
					avalBtnParticles->setID("avalanche-featured-button-particles"_spr);

					avalMenu->addChild(avalBtnParticles);
				};

				bool alwaysCheck = getThisMod->getSettingValue<bool>("check-aval");

				if (alwaysCheck)
				{
					Menu::onCheckForNewAval(nullptr);
				}
				else if (!pinged)
				{
					Menu::onCheckForNewAval(nullptr);

					pinged = true;
				}
				else
				{
					bool isChecked = getThisMod->getSavedValue<bool>("checked-aval-project");

					if (!isChecked)
					{
						m_fields->avalBtnGlow->setVisible(true);
						m_fields->avalBtnMark->setVisible(true);
					}
					else
					{
						m_fields->avalBtnGlow->setVisible(false);
						m_fields->avalBtnMark->setVisible(false);
					};
				};
			}
			else
			{
				log::error("Avalanche featured project button disabled");
			};

			return true;
		}
		else
		{
			return false;
		};
	};

	/*
	vanilla functions
	*/

	void onDaily(CCObject * sender)
	{
		bool alwaysCheck = getThisMod->getSettingValue<bool>("check-aval");

		if (alwaysCheck)
		{
			Menu::onCheckForNewAval(sender);
		};

		MenuLayer::onDaily(sender);
	};

	void onStats(CCObject * sender)
	{
		bool alwaysCheck = getThisMod->getSettingValue<bool>("check-aval");

		if (alwaysCheck)
		{
			Menu::onCheckForNewAval(sender);
		};

		MenuLayer::onStats(sender);
	};

	void onMyProfile(CCObject * sender)
	{
		bool alwaysCheck = getThisMod->getSettingValue<bool>("check-aval");

		if (alwaysCheck)
		{
			Menu::onCheckForNewAval(sender);
		};

		MenuLayer::onMyProfile(sender);
	};

	void onOptions(CCObject * sender)
	{
		bool alwaysCheck = getThisMod->getSettingValue<bool>("check-aval");

		if (alwaysCheck)
		{
			Menu::onCheckForNewAval(sender);
		};

		MenuLayer::onOptions(sender);
	};

	/*
	mod functions
	*/

	void onCheckForNewAval(CCObject *sender)
	{
		m_fields->avalWebListener.bind([this](web::WebTask::Event *e)
									   {
				if (web::WebResponse *avalReqRes = e->getValue())
			{
				std::string avalWebResultUnwrapped = avalReqRes->string().unwrapOr("Uh oh!");
				std::string avalWebResultSaved = getThisMod->getSavedValue<std::string>("aval-project-code");

				bool isChecked = getThisMod->getSavedValue<bool>("checked-aval-project");

				log::debug("Project code '{}' fetched remotely", avalWebResultUnwrapped);

				if (avalWebResultUnwrapped != avalWebResultSaved || !isChecked)
				{
					getThisMod->setSavedValue("checked-aval-project", false);

					m_fields->avalBtnGlow->setVisible(true);
					m_fields->avalBtnMark->setVisible(true);
				}
				else
				{
					m_fields->avalBtnGlow->setVisible(false);
					m_fields->avalBtnMark->setVisible(false);
				};

				getThisMod->setSavedValue("aval-project-code", avalWebResultUnwrapped);
			}
			else if (web::WebProgress *p = e->getProgress())
			{
				log::debug("project code progress: {}", p->downloadProgress().value_or(0.f));
			}
			else if (e->isCancelled())
			{
				log::debug("The Project Code request was cancelled... So sad :(");
			}; });

		auto avalReq = web::WebRequest();
		m_fields->avalWebListener.setFilter(avalReq.get("https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/aval-project/code.txt"));
	};

	void onAvalFeaturedButton(CCObject *)
	{
		AvalancheFeatured::create()->show();

		getThisMod->setSavedValue("checked-aval-project", true);
		m_fields->avalBtnGlow->setVisible(false);
		m_fields->avalBtnMark->setVisible(false);
	};
};