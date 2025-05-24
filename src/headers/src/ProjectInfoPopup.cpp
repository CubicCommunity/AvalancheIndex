#include "../ProjectInfoPopup.hpp"

#include "../../incl/Avalanche.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/cocos.hpp>

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>

using namespace avalanche;
using namespace geode::prelude;

ProjectInfoPopup *ProjectInfoPopup::create()
{
  auto ret = new ProjectInfoPopup;
  if (ret->initAnchored(440, 290))
  {
    ret->autorelease();
    return ret;
  };

  CC_SAFE_DELETE(ret);
  return nullptr;
};

void ProjectInfoPopup::infoPopup(CCObject *)
{
  std::ostringstream typeOfProj;

  switch (m_avalProject.type)
  {
  case Project::Type::TEAM:
    typeOfProj << "a <cg>team project</c> hosted by <cy>" << m_avalProject.host << "</c>";
    break;

  case Project::Type::COLLAB:
    typeOfProj << "a <cb>collaboration project</c> hosted by <cl>Avalanche</c>. One or more guest creators partook in the creation of this level";
    break;

  case Project::Type::EVENT:
    typeOfProj << "an <cs>event level</c>. It is the winner of a public or private event hosted by <cl>Avalanche</c>";
    break;

  case Project::Type::SOLO:
    typeOfProj << "a <co>featured solo level</c>. A member of <cl>Avalanche</c> created this level on their own";
    break;

  default:
    typeOfProj << "an official <cl>Avalanche</c> project";
    break;
  };

  std::ostringstream body;
  body << "<cy>" << m_avalPublisher << "</c> - '<cg>" << m_avalProject.name << "</c>' is " << typeOfProj.str() << ". You can watch its showcase here.";

  std::string resultBody = body.str();

  createQuickPopup(
      m_avalProject.name.c_str(),
      resultBody.c_str(),
      "OK", "Watch",
      [this](auto, bool btn2)
      {
        if (btn2)
        {
          log::info("Opening showcase link in browser: {}", this->m_avalProject.showcase_url);
          web::openLinkInBrowser(this->m_avalProject.showcase_url);
        }
        else
        {
          log::debug("User clicked OK");
        };
      },
      true);
};

void ProjectInfoPopup::onFameInfo(CCObject *)
{

  std::ostringstream body;
  body << "This level, '<cg>" << m_avalProject.name << "</c>', is featured in <cl>Avalanche's</c> <cy>Hall of Fame</c>. It is a special list of levels that are considered to be the best of the best from the team.";

  std::string resultBody = body.str();

  createQuickPopup(
      "Hall of Fame",
      resultBody.c_str(),
      "OK", "Learn More",
      [](auto, bool btn2)
      {
        if (btn2)
        {
          log::info("Opening Hall of Fame link in browser");
          web::openLinkInBrowser("https://avalanche.cubicstudios.xyz/");
        }
        else
        {
          log::debug("User clicked OK");
        };
      },
      true);
};

void ProjectInfoPopup::onPlayShowcase(CCObject *)
{
  std::ostringstream body;
  body << "Watch the full showcase video for <cy>" << m_avalPublisher << "</c> - '<cg>" << m_avalProject.name << "</c>'?";

  std::string resultBody = body.str();

  createQuickPopup(
      m_avalProject.name.c_str(),
      resultBody.c_str(),
      "Cancel", "Watch",
      [this](auto, bool btn2)
      {
        if (btn2)
        {
          log::info("Opening showcase link in browser: {}", this->m_avalProject.showcase_url);
          web::openLinkInBrowser(this->m_avalProject.showcase_url);
        }
        else
        {
          log::debug("User clicked Cancel");
        };
      },
      true);
};

bool ProjectInfoPopup::setup()
{
  setID("project-popup"_spr);
  setTitle("Loading...");

  m_title->setFntFile("bigFont.fnt");
  m_title->setScale(1.25f);

  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();

  // for buttons to work
  m_overlayMenu = CCMenu::create();
  m_overlayMenu->setID("popup-overlay-menu"_spr);
  m_overlayMenu->ignoreAnchorPointForPosition(false);
  m_overlayMenu->setPosition({widthCS / 2.f, heightCS / 2.f});
  m_overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());

  m_mainLayer->addChild(m_overlayMenu);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75f);

  auto infoBtn = CCMenuItemSpriteExtra::create(
      infoBtnSprite,
      this,
      menu_selector(ProjectInfoPopup::infoPopup));
  infoBtn->setID("info-button");
  infoBtn->setPosition({m_mainLayer->getScaledContentWidth() - 15.f, m_mainLayer->getScaledContentHeight() - 15.f});

  m_overlayMenu->addChild(infoBtn);

  log::warn("Project info popup still unfinished, please use ProjectInfoPopup::setProject() to finish setting it up before displaying it");

  return true;
};

ProjectInfoPopup *ProjectInfoPopup::setProject(GJGameLevel *level)
{
  m_level = level;
  m_avalProject = Handler::get()->GetProject(m_level->m_levelID.value());

  if (m_avalProject.type == Project::Type::NONE)
  {
    log::error("Avalanche project type is NONE");
    return this;
  }
  else if (m_avalProject.type == Project::Type::TEAM)
  {
    m_avalPublisher = OFFICIAL_TEAM_NAME;
  }
  else
  {
    m_avalPublisher = m_avalProject.host;
  };

  setTitle(m_avalProject.name);

  auto fameFrame_layout = AxisLayout::create(Axis::Row);
  fameFrame_layout->setCrossAxisLineAlignment(AxisAlignment::Center);
  fameFrame_layout->setCrossAxisAlignment(AxisAlignment::Center);
  fameFrame_layout->setAxisAlignment(AxisAlignment::Center);
  fameFrame_layout->setGrowCrossAxis(false);
  fameFrame_layout->setAxisReverse(false);
  fameFrame_layout->setAutoScale(false);
  fameFrame_layout->setGap(5.f);

  auto fameFrame = CCMenu::create();
  fameFrame->setID("fame-frame"_spr);
  fameFrame->ignoreAnchorPointForPosition(false);
  fameFrame->setPosition({m_mainLayer->getScaledContentWidth() / 2.f, m_title->getPositionY() - 20.f});
  fameFrame->setScaledContentSize({m_mainLayer->getScaledContentWidth() * 0.75f, 12.5f});
  fameFrame->setLayout(fameFrame_layout);

  fameFrame->updateLayout(true);
  m_overlayMenu->addChild(fameFrame);

  if (m_avalProject.fame)
  {
    log::info("Project '{}' is in the Hall of Fame", m_avalProject.name);

    m_title->setFntFile("goldFont.fnt");
    m_title->setScale(1.f);

    m_cornerArtType = "dailyLevelCorner_001.png";

    auto fameIcon = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    fameIcon->setScale(0.25f);

    auto fameLabel = CCLabelBMFont::create("Avalanche Hall of Fame", "goldFont.fnt");
    fameLabel->ignoreAnchorPointForPosition(false);
    fameLabel->setAnchorPoint({0.5, 0.5});
    fameLabel->setScale(0.375f);

    auto fameBtn = CCMenuItemSpriteExtra::create(
        fameLabel,
        this,
        menu_selector(ProjectInfoPopup::onFameInfo));
    fameFrame->addChild(fameIcon);
    fameFrame->addChild(fameBtn);

    fameFrame->updateLayout(true);
  }
  else
  {
    log::debug("Project '{}' is not in the Hall of Fame", m_avalProject.name);
  };

  auto art_bottomLeft = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_bottomLeft->setID("bottom-left-corner");
  art_bottomLeft->setAnchorPoint({0, 0});
  art_bottomLeft->setPosition({0, 0});
  art_bottomLeft->setScale(1.250);
  art_bottomLeft->setFlipX(false);
  art_bottomLeft->setFlipY(false);
  art_bottomLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_bottomLeft);

  auto art_bottomRight = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_bottomRight->setID("bottom-right-corner");
  art_bottomRight->setAnchorPoint({1, 0});
  art_bottomRight->setPosition({m_overlayMenu->getScaledContentWidth(), 0});
  art_bottomRight->setScale(1.250);
  art_bottomRight->setFlipX(true);
  art_bottomRight->setFlipY(false);
  art_bottomLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_bottomRight);

  auto art_topLeft = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_topLeft->setID("top-left-corner");
  art_topLeft->setAnchorPoint({0, 1});
  art_topLeft->setPosition({0, m_overlayMenu->getScaledContentHeight()});
  art_topLeft->setScale(1.250);
  art_topLeft->setFlipX(false);
  art_topLeft->setFlipY(true);
  art_topLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_topLeft);

  auto art_topRight = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_topRight->setID("top-right-corner");
  art_topRight->setAnchorPoint({1, 1});
  art_topRight->setPosition({m_overlayMenu->getScaledContentWidth(), m_overlayMenu->getScaledContentHeight()});
  art_topRight->setScale(1.250);
  art_topRight->setFlipX(true);
  art_topRight->setFlipY(true);
  art_topRight->setZOrder(-1);

  m_overlayMenu->addChild(art_topRight);

  auto comingSoon = CCLabelBMFont::create("More coming soon...", "bigFont.fnt");
  comingSoon->setID("coming-soon-label");
  comingSoon->ignoreAnchorPointForPosition(false);
  comingSoon->setAnchorPoint({0.5, 0.5});
  comingSoon->setPosition({m_mainLayer->getScaledContentWidth() / 2.f, m_mainLayer->getScaledContentHeight() / 2.f});
  comingSoon->setScale(0.25f);

  m_overlayMenu->addChild(comingSoon);

  auto hostLabelTxt = "Published by";

  if (m_avalProject.type == Project::Type::TEAM)
    hostLabelTxt = "Hosted by";

  auto hostName_label = CCLabelBMFont::create(hostLabelTxt, "bigFont.fnt");
  hostName_label->setID("host-name-label");
  hostName_label->ignoreAnchorPointForPosition(false);
  hostName_label->setAnchorPoint({0, 0.5});
  hostName_label->setPosition({10.f, (m_mainLayer->getScaledContentHeight() / 2.f) + 50.f});
  hostName_label->setScale(0.25f);

  auto hostName = CCLabelBMFont::create(m_avalProject.host.c_str(), "goldFont.fnt");
  hostName->setID("host-name");
  hostName->ignoreAnchorPointForPosition(false);
  hostName->setAnchorPoint({0, 0.5});
  hostName->setPosition({10.f, (m_mainLayer->getScaledContentHeight() / 2.f) + 35.f});
  hostName->setScale(0.75f);

  m_overlayMenu->addChild(hostName_label);
  m_overlayMenu->addChild(hostName);

  auto playShowcase_label = CCLabelBMFont::create("Watch the Showcase", "chatFont.fnt");
  playShowcase_label->setID("play-showcase-label");
  playShowcase_label->ignoreAnchorPointForPosition(false);
  playShowcase_label->setAnchorPoint({0.5, 0.5});
  playShowcase_label->setPosition({m_mainLayer->getScaledContentWidth() / 2.f, 60.f});
  playShowcase_label->setScale(1.f);

  auto playShowcase_sprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
  playShowcase_sprite->setScale(0.5f);

  auto playShowcase = CCMenuItemSpriteExtra::create(
      playShowcase_sprite,
      this,
      menu_selector(ProjectInfoPopup::onPlayShowcase));
  playShowcase->setID("play-showcase-button"_spr);
  playShowcase->setPosition({m_mainLayer->getScaledContentWidth() / 2.f, 30.f});

  m_overlayMenu->addChild(playShowcase_label);
  m_overlayMenu->addChild(playShowcase);

  return this;
};

void ProjectInfoPopup::show()
{
  if (m_noElasticity)
    return FLAlertLayer::show();

  GLubyte opacity = getOpacity();
  m_mainLayer->setScale(0.1f);

  m_mainLayer->runAction(
      CCEaseElasticOut::create(CCScaleTo::create(0.3f, 1.0f), 1.6f));

  if (!m_scene)
    m_scene = CCDirector::sharedDirector()->getRunningScene();

  if (!m_ZOrder)
    m_ZOrder = 105;

  m_scene->addChild(this);

  setOpacity(0);
  runAction(CCFadeTo::create(0.14, opacity));
  setVisible(true);
};