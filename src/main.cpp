#include "./Debugger.hpp"

#include "../incl/Avalanche.hpp"

#include "./headers/ParticleHelper.hpp"
#include "./headers/AvalancheFeatured.hpp"

#include "./headers/ProjectInfoPopup.hpp"

#include <string>
#include <chrono>
#include <map>

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>

#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/PauseLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/CommentCell.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/LevelCell.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/PlayLayer.hpp>

using namespace geode::prelude;
using namespace avalanche;

/*
 * Avalanche Index
 *
 * This mod simply handles user and level data from Avalanche to make them look official in-game.
 * All data is remotely fetched.
 *
 * You may find more information on the mod's API functions in the "Avalanche.hpp" file.
 */

 // avalanche data handler
auto getHandler = Handler::get();

// if the server wasn't already checked for the new avalanche project :O
bool noProjectPing = true;

// handles adding avalanche badges to user profiles in the profile page
class $modify(ProfilePage) {
	// modified vanilla loadPageFromUserInfo function
	void loadPageFromUserInfo(GJUserScore * user) {
		ProfilePage::loadPageFromUserInfo(user);

		if (AVAL_GEODE_MOD->getSettingValue<bool>("badge-profile")) {
			// gets all badge data
			getHandler->scanAll();

			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu* cell_menu = typeinfo_cast<CCMenu*>(mLayer->getChildByIDRecursive("username-menu"));

			TextArea* fakeText = nullptr;
			CCLabelBMFont* fakeFont = nullptr;

			Profile plr = getHandler->GetProfile(user->m_accountID);
			getHandler->createBadge(this, plr, cell_menu, fakeText, fakeFont, 0.875f);

			AVAL_LOG_DEBUG("Viewing profile of ID {}", (int)user->m_accountID);
		} else {
			AVAL_LOG_DEBUG("Profile badge disabled");
		};
	};
};

// handles adding avalanche badges to comments in the comment cell
class $modify(CommentCell) {
	// modified vanilla loadFromComment function
	void loadFromComment(GJComment * comment) {
		CommentCell::loadFromComment(comment);

		if (AVAL_GEODE_MOD->getSettingValue<bool>("badge-comments")) {
			// checks if the user is a member of avalanche
			getHandler->scanAll();

			// gets a copy of the main layer
			auto mLayer = m_mainLayer;
			CCMenu* cell_menu = typeinfo_cast<CCMenu*>(mLayer->getChildByIDRecursive("username-menu"));

			auto commentText = dynamic_cast<TextArea*>(m_mainLayer->getChildByID("comment-text-area"));	   // big comment
			auto commentFont = dynamic_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("comment-text-label")); // smol comment

			// checks if commenter published level
			AVAL_LOG_DEBUG("Checking comment on level of ID {}...", (int)comment->m_levelID);

			if (AVAL_GEODE_MOD->getSettingValue<bool>("comments")) {
				if (comment->m_hasLevelID) {
					AVAL_LOG_DEBUG("Comment listed on account page with level linked");
				} else if (auto thisLevel = GameLevelManager::get()->getSavedLevel(comment->m_levelID)) {
					if (comment->m_userID == thisLevel->m_userID.value()) {
						AVAL_LOG_INFO("Commenter {} is level publisher", comment->m_userName);

						commentText = nullptr;
						commentFont = nullptr;
					} else {
						AVAL_LOG_DEBUG("Commenter {} is not level publisher", comment->m_userName);
					};
				} else {
					AVAL_LOG_DEBUG("Comment not published under any level");
				};
			} else {
				commentText = nullptr;
			};

			Profile plr = getHandler->GetProfile(comment->m_accountID);
			getHandler->createBadge(this, plr, cell_menu, commentText, commentFont, 0.55f);

			AVAL_LOG_DEBUG("Viewing comment profile of ID {}", (int)comment->m_accountID);
		} else {
			AVAL_LOG_DEBUG("Comment badge disabled");
		};
	};
};

// attempts to fetch data on level or badge locally to verify ownership of the level
Project::Type scanForLevelCreator(GJGameLevel* level) {
	auto project = getHandler->GetProject(level->m_levelID.value());

	if (project.type == Project::Type::NONE) {
		// get the member's badge data
		auto profile = getHandler->GetProfile(level->m_accountID.value());
		auto badge = profile.badge;
		auto cacheSolo = Handler::badgeStringID[badge];

		// get the badge sprite names
		auto& badgeSpriteName = Handler::badgeSpriteName;
		auto& collabSprite = badgeSpriteName.at(Handler::badgeStringID[Profile::Badge::COLLABORATOR]);
		auto& cubicSprite = badgeSpriteName.at(Handler::badgeStringID[Profile::Badge::CUBIC]);

		// checks if the level is a solo project
		const auto it = badgeSpriteName.find(cacheSolo);
		if (it == badgeSpriteName.end()) {
			AVAL_LOG_ERROR("Key '{}' not found in badgeSpriteName!", cacheSolo);

			return Project::Type::NONE;
		} else {
			const auto& soloSprite = it->second;

			bool notSolo = soloSprite.empty() && (soloSprite != collabSprite) && (soloSprite != cubicSprite);

			bool notPublic = level->m_unlisted || level->m_friendsOnly;

			// must be public
			if (notPublic) {
				AVAL_LOG_ERROR("Level {} is unlisted", (int)level->m_levelID.value());

				return Project::Type::NONE;
			} else {
				AVAL_LOG_DEBUG("Level {} is publicly listed!", (int)level->m_levelID.value());

				// checks if owned by publisher account
				if (level->m_accountID.value() == ACC_PUBLISHER) {
					AVAL_LOG_DEBUG("Level {} is Avalanche team project", (int)level->m_levelID.value());

					return Project::Type::TEAM;
				} else {
					// checks if level is published by a team member
					if (notSolo) {
						AVAL_LOG_ERROR("Level {} not associated with Avalanche", (int)level->m_levelID.value());

						return Project::Type::NONE;
					} else {
						// checks if level is rated
						if (level->m_stars.value() >= 1) {
							AVAL_LOG_DEBUG("Level {} is Avalanche team member solo", (int)level->m_levelID.value());

							return Project::Type::SOLO;
						} else {
							AVAL_LOG_ERROR("Level {} is unrated", (int)level->m_levelID.value());

							return Project::Type::NONE;
						};
					};
				};
			};
		};
	} else {
		return project.type;
	};
};

// handles the level info layer for avalanche featured projects
class $modify(LevelInfo, LevelInfoLayer) {
	// modified vanilla init function
	bool init(GJGameLevel * level, bool challenge) {
		if (LevelInfoLayer::init(level, challenge)) {
			// check display settings
			bool displaySoloLayers = AVAL_GEODE_MOD->getSettingValue<bool>("solo-layers");
			bool displayTeamLayers = AVAL_GEODE_MOD->getSettingValue<bool>("team-layers");
			bool displayEventLayers = AVAL_GEODE_MOD->getSettingValue<bool>("event-layers");
			bool displayCollabLayers = AVAL_GEODE_MOD->getSettingValue<bool>("collab-layers");

			// get main bg color layer
			auto bg = this->getChildByID("background");
			auto background = as<CCSprite*>(bg);

			// get level name node
			auto nameText = this->getChildByID("title-label");
			auto levelName = dynamic_cast<CCLabelBMFont*>(nameText);

			// whether or not display for classics only
			bool onlyClassic = AVAL_GEODE_MOD->getSettingValue<bool>("classic-only") && level->isPlatformer();

			CCMenu* leftMenu = typeinfo_cast<CCMenu*>(this->getChildByID("left-side-menu"));

			Project thisProj = getHandler->GetProject(level->m_levelID.value());

			// if the project is not avalanche, then we don't need to do anything
			if (thisProj.type == Project::Type::NONE) {
				AVAL_LOG_ERROR("Level {} is not an Avalanche project", (int)level->m_levelID.value());
			} else {
				auto showProjectInfo = AVAL_GEODE_MOD->getSettingValue<bool>("show-proj-info");

				if (showProjectInfo) {
					CCSprite* avalBtnSprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
					avalBtnSprite->setScale(0.944f);

					CCSprite* avalBtnSpriteIcon = CCSprite::createWithSpriteFrameName("button-logo.png"_spr);
					avalBtnSpriteIcon->setPositionX(avalBtnSprite->getContentWidth() / 2.025f);
					avalBtnSpriteIcon->setPositionY(avalBtnSprite->getContentHeight() / 2.f);
					avalBtnSpriteIcon->ignoreAnchorPointForPosition(false);
					avalBtnSpriteIcon->setAnchorPoint({ 0.5, 0.5 });
					avalBtnSpriteIcon->setScale(0.875f);

					avalBtnSprite->addChild(avalBtnSpriteIcon);

					CCMenuItemSpriteExtra* avalBtn = CCMenuItemSpriteExtra::create(
						avalBtnSprite,
						this,
						menu_selector(LevelInfo::onAvalancheButton));
					avalBtn->setID("avalanche-button"_spr);
					avalBtn->setZOrder(10);

					leftMenu->addChild(avalBtn);
					leftMenu->updateLayout();
				} else {
					AVAL_LOG_WARN("Project info button not shown, setting up level info layer");
				};
			};

			auto levelType = scanForLevelCreator(level);

			// check the project type and set decoration accordingly
			if (levelType == Project::Type::SOLO) {
				if (displaySoloLayers) {
					if (onlyClassic) {
						AVAL_LOG_ERROR("Solo level {} is platformer", (int)level->m_levelID.value());
					} else {
						LevelInfo::setSoloDisplay(background, thisProj.fame);
					};
				} else {
					AVAL_LOG_WARN("Solo layers not displayed, setting up level info layer");
				};
			} else if (levelType == Project::Type::TEAM) {
				if (displayTeamLayers) {
					if (level->isPlatformer()) {
						AVAL_LOG_ERROR("Team level {} is platformer", (int)level->m_levelID.value());
					} else if (level->m_unlisted) {
						AVAL_LOG_ERROR("Team level {} is unlisted", (int)level->m_levelID.value());
					} else {
						LevelInfo::setTeamDisplay(background, levelName);
					};
				} else {
					AVAL_LOG_WARN("Team layers not displayed, setting up level info layer");
				};
			} else if (levelType == Project::Type::EVENT) {
				if (displayEventLayers) {
					LevelInfo::setEventDisplay(background, thisProj.fame);
				} else {
					AVAL_LOG_WARN("Event layers not displayed, setting up level info layer");
				};
			} else if (levelType == Project::Type::COLLAB) {
				if (displayCollabLayers) {
					LevelInfo::setCollabDisplay(background, thisProj.fame);
				} else {
					AVAL_LOG_WARN("Collaboration layers not displayed, setting up level info layer");
				};
			} else {
				AVAL_LOG_ERROR("Level {} is not an Avalanche project", (int)level->m_levelID.value());
			};

			return true;
		} else {
			return false;
		};
	};

	// set solo project decoration for the level info layer
	void setSoloDisplay(CCSprite * background, bool fame = false) {
		if (background) {
			background->setColor({ 70, 77, 117 });

			if (fame) LevelInfo::setFame(background);
		} else {
			AVAL_LOG_ERROR("Cannot set solo display with missing background");
		};
	};

	// set collaboration project decoration for the level info layer
	void setCollabDisplay(CCSprite * background, bool fame = false) {
		if (background) {
			background->setColor({ 148, 154, 189 });

			if (fame) LevelInfo::setFame(background);
		} else {
			AVAL_LOG_ERROR("Cannot set collaboration display with missing background");
		};
	};

	// set team project decoration for the level info layer
	void setTeamDisplay(CCSprite * background, CCLabelBMFont * levelName) {
		if ((background) && (levelName)) {
			if (auto bgSprite = CCSprite::createWithSpriteFrameName("game_bg_19_001.png")) {
				bgSprite->setColor({ 66, 94, 255 });
				bgSprite->setAnchorPoint({ 0.5, 0.5 });
				bgSprite->ignoreAnchorPointForPosition(false);
				bgSprite->setContentSize({ this->getScaledContentWidth(), this->getScaledContentWidth() });
				bgSprite->setPosition({ this->getScaledContentWidth() / 2, this->getScaledContentHeight() / 2 });
				bgSprite->setZOrder(background->getZOrder());
				bgSprite->setID("team_background"_spr);

				if (auto bgThumbnail = CCSprite::createWithSpriteFrameName("project-bg.png"_spr)) {
					bgThumbnail->setOpacity(75);
					bgThumbnail->setPosition({ 0, 0 });
					bgThumbnail->setAnchorPoint({ 0, 0 });
					bgThumbnail->ignoreAnchorPointForPosition(false);
					bgThumbnail->setZOrder(background->getZOrder() + 1);
					bgThumbnail->setID("team_thumbnail"_spr);

					auto ogHeight = this->getContentHeight();
					auto thumbHeight = bgThumbnail->getContentHeight();

					// checks if the heights are valid before rescaling
					if (ogHeight <= 0 || thumbHeight <= 0) {
						AVAL_LOG_ERROR("Invalid dimensions for scaling: bgSprite or bgThumbnail has zero height");
					} else {
						float scaleFactor = ogHeight / thumbHeight;
						bgThumbnail->setScale(scaleFactor);
					};

					this->addChild(bgThumbnail);
				} else {
					AVAL_LOG_ERROR("Failed to load sprite: project-bg.png");
				};

				this->addChild(bgSprite);
			} else {
				AVAL_LOG_ERROR("Failed to load sprite: game_bg_19_001.png");
			};

			background->setColor({ 66, 94, 255 });
			background->setZOrder(-5);

			auto levelNameOgWidth = levelName->getScaledContentWidth();
			levelName->setFntFile("gjFont59.fnt");

			if (levelNameOgWidth <= 0) {
				AVAL_LOG_ERROR("Invalid level name dimensions: Original width is zero");
			} else {
				auto scaleDownBy = levelNameOgWidth / levelName->getScaledContentWidth();
				levelName->setScale(levelName->getScale() * scaleDownBy);
			};
		} else {
			AVAL_LOG_ERROR("Failed to set team display: background or level name is null");
		};
	};

	// set event project decoration for the level info layer
	void setEventDisplay(CCSprite * background, bool fame = false) {
		if (background) {
			background->setColor({ 211, 207, 0 });

			if (fame) LevelInfo::setFame(background);
		} else {
			AVAL_LOG_ERROR("Cannot set event display with missing background");
		};
	};

	// set hall of fame decoration for the level info layer
	void setFame(CCSprite * background) {
		if (background) {
			bool showFame = AVAL_GEODE_MOD->getSettingValue<bool>("show-fame");

			if (showFame) {
				if (auto bgThumbnail = CCSprite::createWithSpriteFrameName("fame-bg.png"_spr)) {
					bgThumbnail->setOpacity(75);
					bgThumbnail->setAnchorPoint({ 0.5, 0 });
					bgThumbnail->ignoreAnchorPointForPosition(false);
					bgThumbnail->setPosition({ this->getContentWidth() / 2, 0 });
					bgThumbnail->setZOrder(background->getZOrder() + 1);
					bgThumbnail->setID("team_thumbnail"_spr);

					auto ogWidth = this->getContentWidth();
					auto scaledWidth = bgThumbnail->getContentWidth();

					if (ogWidth <= 0 || scaledWidth <= 0) {
						AVAL_LOG_ERROR("Invalid dimensions for scaling: this or bgThumbnail has zero width");
					} else {
						float scaleFactor = ogWidth / scaledWidth;
						bgThumbnail->setScale(scaleFactor);

						background->setZOrder(-5);
					};

					this->addChild(bgThumbnail);
				} else {
					AVAL_LOG_ERROR("Failed to load sprite: fame-bg.png");
				};
			} else {
				AVAL_LOG_ERROR("Display of hall of fame effect disabled");
			};
		} else {
			AVAL_LOG_ERROR("Cannot set hall of fame display with missing background");
		};
	};

	// when player presses the avalanche button
	void onAvalancheButton(CCObject * sender) {
		ProjectInfoPopup::create()->setProject(this->m_level)->show();
	};
};

// handles the level cell for Avalanche featured projects
class $modify(Level, LevelCell) {
	// modified vanilla loadFromLevel function
	void loadFromLevel(GJGameLevel * level) {
		LevelCell::loadFromLevel(level);

		// check display settings
		bool displaySoloCells = AVAL_GEODE_MOD->getSettingValue<bool>("solo-cells");
		bool displayTeamCells = AVAL_GEODE_MOD->getSettingValue<bool>("team-cells");
		bool displayEventCells = AVAL_GEODE_MOD->getSettingValue<bool>("event-cells");
		bool displayCollabCells = AVAL_GEODE_MOD->getSettingValue<bool>("collab-cells");

		// get main bg color layer
		auto color = this->getChildByType<CCLayerColor>(0);

		// get level name text
		auto nameText = m_mainLayer->getChildByID("level-name");
		auto levelName = as<CCLabelBMFont*>(nameText);

		// whether or not display for classics only
		bool onlyClassic = AVAL_GEODE_MOD->getSettingValue<bool>("classic-only") && level->isPlatformer();

		if ((color) && (levelName)) {
			// checks if the level is an avalanche project
			auto lvl = getHandler->GetProject(level->m_levelID.value());
			auto levelType = scanForLevelCreator(level);

			// check the project type and set decoration accordingly
			if (levelType == Project::Type::SOLO) {
				if (displaySoloCells) {
					if (onlyClassic) {
						AVAL_LOG_ERROR("Solo level {} is platformer", (int)level->m_levelID.value());
					} else {
						Level::setSoloDisplay(color, lvl.fame);
					};
				} else {
					AVAL_LOG_WARN("Solo cells not displayed, setting up level cell");
				};
			} else if (levelType == Project::Type::TEAM) {
				if (displayTeamCells) {
					if (level->isPlatformer()) {
						AVAL_LOG_ERROR("Team level {} is platformer", (int)level->m_levelID.value());
					} else if (level->m_unlisted) {
						AVAL_LOG_ERROR("Team level {} is unlisted", (int)level->m_levelID.value());
					} else {
						Level::setTeamDisplay(color, levelName, lvl.fame);
					};
				} else {
					AVAL_LOG_WARN("Team cells not displayed, setting up level cell");
				};
			} else if (levelType == Project::Type::EVENT) {
				if (displayEventCells) {
					Level::setEventDisplay(color, lvl.fame);
				} else {
					AVAL_LOG_WARN("Event cells not displayed, setting up level cell");
				};
			} else if (levelType == Project::Type::COLLAB) {
				if (displayCollabCells) {
					Level::setCollabDisplay(color, lvl.fame);
				} else {
					AVAL_LOG_WARN("Collaboration cells not displayed, setting up level cell");
				};
			} else {
				AVAL_LOG_ERROR("Level {} is not an Avalanche project", (int)level->m_levelID.value());
			};
		} else {
			AVAL_LOG_ERROR("ccColor3B not found!");
		};
	};

	// set solo project decoration for level cell
	void setSoloDisplay(CCLayerColor * colorNode, bool fame = false) {
		if (colorNode) {
			if (auto newColor = CCLayerColor::create({ 70, 77, 117, 255 })) {
				newColor->setScaledContentSize(colorNode->getScaledContentSize());
				newColor->setAnchorPoint(colorNode->getAnchorPoint());
				newColor->setPosition(colorNode->getPosition());
				newColor->setZOrder(colorNode->getZOrder() - 2);
				newColor->setScale(colorNode->getScale());
				newColor->setID("solo_color"_spr);

				colorNode->removeMeAndCleanup();
				this->addChild(newColor);

				if (fame) Level::setFame(newColor, { 255, 255, 255 });
			} else {
				AVAL_LOG_ERROR("Failed to create solo display color layer");
			};
		} else {
			AVAL_LOG_ERROR("Cannot set solo display with missing color node");
		};
	};

	// set collaboration project decoration for level cell
	void setCollabDisplay(CCLayerColor * colorNode, bool fame = false) {
		if (colorNode) {
			if (auto newColor = CCLayerColor::create({ 148, 154, 189, 255 })) {
				newColor->setScaledContentSize(colorNode->getScaledContentSize());
				newColor->setAnchorPoint(colorNode->getAnchorPoint());
				newColor->setPosition(colorNode->getPosition());
				newColor->setZOrder(colorNode->getZOrder() - 2);
				newColor->setScale(colorNode->getScale());
				newColor->setID("collab_color"_spr);

				colorNode->removeMeAndCleanup();
				this->addChild(newColor);

				if (fame) Level::setFame(newColor, { 255, 255, 255 });
			} else {
				AVAL_LOG_ERROR("Failed to create collaboration display color layer");
			};
		} else {
			AVAL_LOG_ERROR("Cannot set collaboration display with missing color node");
		};
	};

	// set team project decoration for level cell
	void setTeamDisplay(CCLayerColor * colorNode, CCLabelBMFont * levelName, bool fame = false) {
		if ((colorNode) && (levelName)) {
			if (auto newColor = CCLayerColor::create({ 66, 94, 255, 255 })) {
				newColor->setScaledContentSize(colorNode->getScaledContentSize());
				newColor->setAnchorPoint(colorNode->getAnchorPoint());
				newColor->setPosition(colorNode->getPosition());
				newColor->setZOrder(colorNode->getZOrder() - 2);
				newColor->setScale(colorNode->getScale());
				newColor->setID("team_color"_spr);

				auto levelNameOgWidth = levelName->getScaledContentWidth();

				levelName->setFntFile("gjFont59.fnt");

				auto scaleDownBy = (levelNameOgWidth / levelName->getScaledContentWidth());

				levelName->setScale(levelName->getScale() * scaleDownBy);

				colorNode->removeMeAndCleanup();
				this->addChild(newColor);

				if (fame) Level::setFame(newColor, { 255, 244, 95 });
			} else {
				AVAL_LOG_ERROR("Failed to create team display color layer");
			};
		} else {
			AVAL_LOG_ERROR("Failed to set team display: color node or level name is null");
		};
	};

	// set event project decoration for level cell
	void setEventDisplay(CCLayerColor * colorNode, bool fame = false) {
		if (colorNode) {
			if (auto newColor = CCLayerColor::create({ 211, 207, 0, 255 })) {
				newColor->setScaledContentSize(colorNode->getScaledContentSize());
				newColor->setAnchorPoint(colorNode->getAnchorPoint());
				newColor->setPosition(colorNode->getPosition());
				newColor->setZOrder(colorNode->getZOrder() - 2);
				newColor->setScale(colorNode->getScale());
				newColor->setID("event_color"_spr);

				colorNode->removeMeAndCleanup();
				this->addChild(newColor);

				if (fame) Level::setFame(newColor, { 85, 249, 255 });
			} else {
				AVAL_LOG_ERROR("Failed to create event display color layer");
			};
		} else {
			AVAL_LOG_ERROR("Cannot set event display with missing color node");
		};
	};

	// set hall of fame decoration for level cell
	void setFame(CCLayerColor * newColor, ccColor3B glow = { 255, 255, 255 }) {
		if (newColor) {
			bool showFame = AVAL_GEODE_MOD->getSettingValue<bool>("show-fame");

			if (showFame) {
				if (auto fameGlow = CCSprite::create("fame-glow.png"_spr)) {
					fameGlow->setZOrder(newColor->getZOrder() + 1);
					fameGlow->ignoreAnchorPointForPosition(false);
					fameGlow->setAnchorPoint({ 0, 0 });
					fameGlow->setPosition({ 0, 0 });
					fameGlow->setColor(glow);
					fameGlow->setID("fame"_spr);

					// for compact lists
					float reScale = (this->m_height / fameGlow->getContentHeight()) * this->getScale();
					fameGlow->setScale(reScale);

					this->addChild(fameGlow);
				} else {
					AVAL_LOG_ERROR("Failed to load sprite: fame-glow.png");
				};
			} else {
				AVAL_LOG_ERROR("Display of famed effect disabled");
			};
		} else {
			AVAL_LOG_ERROR("Cannot set hall of fame display with missing color node or glow color");
		};
	};
};

// add avalanche featured project button to the pause menu
class $modify(Pause, PauseLayer) {
	struct Fields {
		GJGameLevel* m_level = PlayLayer::get()->m_level; // level to show info for
	};

	// modified vanilla customSetup function
	void customSetup() {
		PauseLayer::customSetup();

		if (auto rightMenu = this->getChildByID("right-button-menu")) {
			if (m_fields->m_level) {
				Project thisProj = getHandler->GetProject(m_fields->m_level->m_levelID.value());

				if (thisProj.type == Project::Type::NONE) {
					AVAL_LOG_ERROR("Level {} is not an Avalanche project", (int)m_fields->m_level->m_levelID.value());
				} else {
					auto showProjectInfo = AVAL_GEODE_MOD->getSettingValue<bool>("show-proj-info");

					if (showProjectInfo) {
						CCSprite* avalBtnSprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
						avalBtnSprite->setScale(0.6f);

						CCSprite* avalBtnSpriteIcon = CCSprite::createWithSpriteFrameName("button-logo.png"_spr);
						avalBtnSpriteIcon->setPositionX(avalBtnSprite->getContentWidth() / 2.025f);
						avalBtnSpriteIcon->setPositionY(avalBtnSprite->getContentHeight() / 2.f);
						avalBtnSpriteIcon->ignoreAnchorPointForPosition(false);
						avalBtnSpriteIcon->setAnchorPoint({ 0.5, 0.5 });
						avalBtnSpriteIcon->setScale(0.875f);

						avalBtnSprite->addChild(avalBtnSpriteIcon);

						CCMenuItemSpriteExtra* avalBtn = CCMenuItemSpriteExtra::create(
							avalBtnSprite,
							this,
							menu_selector(Pause::onAvalancheButton));
						avalBtn->setID("avalanche-button"_spr);
						avalBtn->setZOrder(10);

						rightMenu->addChild(avalBtn);
						rightMenu->updateLayout();
					} else {
						AVAL_LOG_WARN("Project info button not shown, setting up pause menu");
					};
				};
			} else {
				AVAL_LOG_ERROR("Pause menu cannot find current level");
			};
		} else {
			AVAL_LOG_ERROR("Pause menu cannot find right button menu");
		};
	};

	// when player presses the avalanche button
	void onAvalancheButton(CCObject * sender) {
		ProjectInfoPopup::create()->setProject(m_fields->m_level)->show();
	};
};

// everything to do with the avalanche featured project button pretty much
class $modify(Menu, MenuLayer) {
	struct Fields {
		EventListener<web::WebTask> avalWebListener; // web listener for avalanche featured project

		CCSprite* avalBtnGlow = nullptr; // glow sprite for avalanche featured button
		CCSprite* avalBtnMark = nullptr; // unread mark sprite for avalanche featured button
	};

	// modified vanilla init function
	bool init() {
		if (MenuLayer::init()) {
			std::string ver = AVAL_GEODE_MOD->getVersion().toVString();
			AVAL_LOG_DEBUG("Loaded mod version {}", ver);

			bool viewChangelog = AVAL_GEODE_MOD->getSettingValue<bool>("view-changelog");

			if (viewChangelog) {
				std::string changelogVer = AVAL_GEODE_MOD->getSavedValue<std::string>("changelog-ver");

				if (changelogVer.empty()) {
					AVAL_LOG_DEBUG("No changelog version found, setting to {}", ver);
					changelogVer = AVAL_GEODE_MOD->setSavedValue<std::string>("changelog-ver", ver);
				} else {
					AVAL_LOG_INFO("Changelog version {} found", changelogVer);

					// check if the changelog was already shown
					if (changelogVer == ver) {
						AVAL_LOG_DEBUG("Changelog already shown for version {}", ver);
					} else {
						createQuickPopup(
							"Avalanche Index Updated",
							"Check out the new features in the changelog!",
							"Cancel", "OK",
							[](auto, bool btn2) {
								if (btn2) {
									openChangelogPopup(AVAL_GEODE_MOD);
								} else {
									AVAL_LOG_DEBUG("User clicked Cancel");
								}; },
							true);

						AVAL_GEODE_MOD->setSavedValue<std::string>("changelog-ver", ver);
						AVAL_GEODE_MOD->setSavedValue<bool>("checked-aval-project", false);
					};
				};
			} else {
				AVAL_LOG_DEBUG("Changelog alert disabled");
			};

			auto winSizeX = this->getScaledContentWidth();
			auto winSizeY = this->getScaledContentHeight();

			bool showAvalButton = AVAL_GEODE_MOD->getSettingValue<bool>("show-aval-featured");

			if (showAvalButton) {
				auto avalMenu = CCMenu::create();
				avalMenu->ignoreAnchorPointForPosition(false);
				avalMenu->setPosition({ winSizeX / 2, (winSizeY / 2) - 70.f });
				avalMenu->setScaledContentSize({ winSizeX - 75.f, 50.f });

				this->addChild(avalMenu);

				auto avalBtnSprite = CCSprite::createWithSpriteFrameName("button-logo-framed.png"_spr);
				avalBtnSprite->ignoreAnchorPointForPosition(false);
				avalBtnSprite->setScale(0.75f);

				// create the avalanche featured button
				auto avalBtn = CCMenuItemSpriteExtra::create(
					avalBtnSprite,
					this,
					menu_selector(Menu::onAvalFeaturedButton));
				avalBtn->setPosition({ avalMenu->getScaledContentWidth() / 2, avalMenu->getScaledContentHeight() / 2 });
				avalBtn->setID("avalanche-featured-button"_spr);
				avalBtn->ignoreAnchorPointForPosition(false);

				avalMenu->addChild(avalBtn);

				// featured ring
				if (m_fields->avalBtnGlow) {
					AVAL_LOG_ERROR("Avalanche featured button glow sprite already initialized");
				} else {
					m_fields->avalBtnGlow = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");

					m_fields->avalBtnGlow->setID("featuredRing"_spr);
					m_fields->avalBtnGlow->setScale(1.25f);
					m_fields->avalBtnGlow->ignoreAnchorPointForPosition(false);
					m_fields->avalBtnGlow->setPosition({ avalBtnSprite->getScaledContentWidth() / 2, (avalBtnSprite->getScaledContentHeight() / 2) * 0.65f });
					m_fields->avalBtnGlow->setAnchorPoint({ 0.5f, 0.5f });
					m_fields->avalBtnGlow->setZOrder(-1);
					m_fields->avalBtnGlow->setVisible(false);

					if (auto created = avalBtn->getChildByID("featuredRing"_spr)) {
						auto markID = created->getID().c_str();
						AVAL_LOG_ERROR("Avalanche featured button glow sprite already exists: {}", markID);
					} else {
						avalBtn->addChild(m_fields->avalBtnGlow);
					};
				};

				// unread mark
				if (m_fields->avalBtnMark) {
					AVAL_LOG_ERROR("Avalanche featured button mark sprite already initialized");
				} else {
					m_fields->avalBtnMark = CCSprite::createWithSpriteFrameName("exMark_001.png");
					m_fields->avalBtnMark->setID("notifMark"_spr);
					m_fields->avalBtnMark->setScale(0.5f);
					m_fields->avalBtnMark->ignoreAnchorPointForPosition(false);
					m_fields->avalBtnMark->setPosition({ avalBtn->getScaledContentWidth() * 0.875f, avalBtn->getScaledContentHeight() * 0.875f });
					m_fields->avalBtnMark->setAnchorPoint({ 0.5f, 0.5f });
					m_fields->avalBtnMark->setVisible(false);

					if (auto created = avalBtn->getChildByID("notifMark"_spr)) {
						auto markID = created->getID().c_str();
						AVAL_LOG_ERROR("Avalanche featured button mark sprite already exists: {}", markID);
					} else {
						avalBtn->addChild(m_fields->avalBtnMark);
					};
				};

				try {
					// add particles on the featured button
					if (CCParticleSystem* avalBtnParticles = ParticleHelper::createAvalFeaturedParticles(100.0f)) {
						avalBtnParticles->setPosition(avalBtn->getPosition());
						avalBtnParticles->setAnchorPoint({ 0.5f, 0.5f });
						avalBtnParticles->setScale(1.0f);
						avalBtnParticles->setZOrder(-2);
						avalBtnParticles->setStartColor({ 6, 2, 32, 255 });
						avalBtnParticles->setEndColor({ 33, 33, 33, 100 });
						avalBtnParticles->setEmissionRate(20.f);
						avalBtnParticles->setRotatePerSecond(22.5f);
						avalBtnParticles->setStartSize(5.f);
						avalBtnParticles->setEndSize(1.25f);
						avalBtnParticles->setID("button-particles"_spr);

						avalMenu->addChild(avalBtnParticles);
					} else {
						AVAL_LOG_ERROR("Failed to create Avalanche featured button particles");
					};
				} catch (std::exception& e) {
					AVAL_LOG_ERROR("Failed to create Avalanche featured button particles: {}", e.what());
				};

				bool alwaysCheck = AVAL_GEODE_MOD->getSettingValue<bool>("check-aval");

				CCObject* fakeObj = nullptr;

				if (alwaysCheck) {
					Menu::onCheckForNewAval(fakeObj);
				} else if (noProjectPing) {
					Menu::onCheckForNewAval(fakeObj);

					noProjectPing = false;
				} else {
					bool isChecked = AVAL_GEODE_MOD->getSavedValue<bool>("checked-aval-project");

					if (!isChecked) {
						if (m_fields->avalBtnGlow) m_fields->avalBtnGlow->setVisible(true);
						if (m_fields->avalBtnMark) m_fields->avalBtnMark->setVisible(true);
					} else {
						if (m_fields->avalBtnGlow) m_fields->avalBtnGlow->setVisible(false);
						if (m_fields->avalBtnMark) m_fields->avalBtnMark->setVisible(false);
					};
				};
			} else {
				AVAL_LOG_ERROR("Avalanche featured project button disabled");
			};

			// gets all badge data
			getHandler->scanAll();

			return true;
		} else {
			return false;
		};
	};

	/*
	vanilla functions
	*/

	void onDaily(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onDaily(sender);
	};

	void onStats(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onStats(sender);
	};

	void onMyProfile(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onMyProfile(sender);
	};

	void onOptions(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onOptions(sender);
	};

	void onMoreGames(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onMoreGames(sender);
	};

	void onGarage(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onGarage(sender);
	};

	void onCreator(CCObject * sender) {
		Menu::quickCheck(sender);
		MenuLayer::onCreator(sender);
	};

	/*
	mod functions
	*/

	// checks for aval project updates
	void quickCheck(CCObject * sender) {
		bool alwaysCheck = AVAL_GEODE_MOD->getSettingValue<bool>("check-aval");

		if (alwaysCheck) {
			Menu::onCheckForNewAval(sender);
		} else {
			AVAL_LOG_DEBUG("Avalanche project check skipped");
		};
	};

	// pings the server to check if a new aval project is available
	void onCheckForNewAval(CCObject * sender) {
		bool avalButton = AVAL_GEODE_MOD->getSettingValue<bool>("show-aval-featured");

		if (avalButton) {
			m_fields->avalWebListener.bind([this](web::WebTask::Event* e) {
				if (web::WebResponse* avalReqRes = e->getValue()) {
					if (avalReqRes->ok()) {
						if (avalReqRes->string().isOk()) {
							try {
								std::string avalWebResultUnwrapped = avalReqRes->string().unwrapOr("Uh oh!");
								std::string avalWebResultSaved = AVAL_GEODE_MOD->getSavedValue<std::string>("aval-project-code");

								bool isChecked = AVAL_GEODE_MOD->getSavedValue<bool>("checked-aval-project");

								AVAL_LOG_DEBUG("Project code '{}' fetched remotely", avalWebResultUnwrapped);

								if ((avalWebResultUnwrapped == avalWebResultSaved) || isChecked) {
									if (m_fields->avalBtnGlow) m_fields->avalBtnGlow->setVisible(false);
									if (m_fields->avalBtnMark) m_fields->avalBtnMark->setVisible(false);
								} else {
									AVAL_GEODE_MOD->setSavedValue("checked-aval-project", false);

									if (m_fields->avalBtnGlow) m_fields->avalBtnGlow->setVisible(true);
									if (m_fields->avalBtnMark) m_fields->avalBtnMark->setVisible(true);
								};

								AVAL_GEODE_MOD->setSavedValue("aval-project-code", avalWebResultUnwrapped);
							} catch (std::exception& e) {
								AVAL_LOG_ERROR("Error processing Avalanche project code: {}", e.what());

								if (AVAL_GEODE_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Error processing Avalanche project code", NotificationIcon::Error, 2.5f)->show();
							};
						} else {
							AVAL_LOG_ERROR("Failed to fetch Avalanche featured project code");
						};
					} else {
						AVAL_LOG_ERROR("Unable to check server for new Avalanche featured project");

						if (AVAL_GEODE_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch featured project", NotificationIcon::Error, 2.5f)->show();
					};
				} else if (web::WebProgress* p = e->getProgress()) {
					AVAL_LOG_DEBUG("Avalanche project code progress: {}", (float)p->downloadProgress().value_or(0.f));
				} else if (e->isCancelled()) {
					AVAL_LOG_DEBUG("Unable to check server for new Avalanche featured project");

					if (AVAL_GEODE_MOD->getSettingValue<bool>("err-notifs")) Notification::create("Unable to fetch featured project", NotificationIcon::Error, 2.5f)->show();
				}; });

				auto avalReq = web::WebRequest();
				m_fields->avalWebListener.setFilter(avalReq.get("https://gh.cubicstudios.xyz/WebLPS/aval-project/code.txt"));
		} else {
			AVAL_LOG_ERROR("Avalanche featured project button disabled");
		};
	};

	// shows the popup when pressing the avalanche button
	void onAvalFeaturedButton(CCObject*) {
		AvalancheFeatured::create()->show();

		AVAL_GEODE_MOD->setSavedValue("checked-aval-project", true);

		if (m_fields->avalBtnGlow) m_fields->avalBtnGlow->setVisible(false);
		if (m_fields->avalBtnMark) m_fields->avalBtnMark->setVisible(false);
	};
};