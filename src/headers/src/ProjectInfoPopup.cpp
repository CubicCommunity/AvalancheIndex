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
  auto hosted = m_avalProject.host;
  std::ostringstream typeOfProj;

  switch (m_avalProject.type)
  {
  case Project::Type::TEAM:
    hosted = "Avalanche";
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

  std::ostringstream body; // for ios
  body << "<cy>" << std::string(hosted) << "</c> - <cg>'" << m_avalProject.name << "'</c> is " << typeOfProj.str() << ". You can watch its showcase here.";

  std::string resultBody = body.str();

  createQuickPopup(
      m_avalProject.name.c_str(),
      resultBody.c_str(),
      "OK", "Watch",
      [this](auto, bool btn2)
      {
        if (btn2)
        {
          web::openLinkInBrowser(this->m_avalProject.showcase_url);
        };
      },
      true);
};

bool ProjectInfoPopup::setup()
{
  setID("project-popup"_spr);
  setTitle("Loading...");
  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();
  const auto buttons_height = 0.82f * heightCS;

  // for buttons to work
  CCMenu *overlayMenu = CCMenu::create();
  overlayMenu->setID("popup-overlay-menu"_spr);
  overlayMenu->ignoreAnchorPointForPosition(false);
  overlayMenu->setPosition(widthCS / 2, heightCS / 2);
  overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());

  m_mainLayer->addChild(overlayMenu);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75);

  auto infoBtn = CCMenuItemSpriteExtra::create(
      infoBtnSprite,
      this,
      menu_selector(ProjectInfoPopup::infoPopup));
  infoBtn->setID("info-button");
  infoBtn->setPosition(m_mainLayer->getScaledContentWidth() - 17.5f, m_mainLayer->getScaledContentHeight() - 17.5f);

  overlayMenu->addChild(infoBtn);

  return true;
};

ProjectInfoPopup *ProjectInfoPopup::setProject(GJGameLevel *level)
{
  log::warn("Project info popup still unfinished");

  m_level = level;
  m_avalProject = Handler::get().GetProject(m_level->m_levelID.value());

  setTitle(m_avalProject.name);

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