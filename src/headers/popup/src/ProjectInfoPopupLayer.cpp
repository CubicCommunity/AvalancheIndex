#include "../ProjectInfoPopupLayer.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/cocos.hpp>

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

ProjectInfoPopupLayer *ProjectInfoPopupLayer::create()
{
  auto ret = new ProjectInfoPopupLayer;
  if (ret->initAnchored(440, 290))
  {
    ret->autorelease();
    return ret;
  };

  delete ret;
  return nullptr;
};

void ProjectInfoPopupLayer::infoPopup(CCObject *)
{
  FLAlertLayer::create(
      "Example Pop-up Layer",
      "<cr>Developer.</c> This is an <cy>example pop-up layer</c> for the ease of <cp>OGDPS</c> developement. This pop-up is here to serve as a template if more pop-ups are to be created.",
      "OK")
      ->show();
};

bool ProjectInfoPopupLayer::setup()
{
  setID("CustomPopup"_spr);
  setTitle("Custom Popup Layer");
  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();
  const auto buttons_height = 0.82f * heightCS;

  auto bg2_layer = CCLayerColor::create();
  bg2_layer->setColor({});
  bg2_layer->setOpacity(75);
  bg2_layer->setPosition({45, 13});
  bg2_layer->setContentSize({widthCS, heightCS});
  m_mainLayer->addChild(bg2_layer);

  // for buttons to work
  CCMenu *overlayMenu = CCMenu::create();
  overlayMenu->setID("overlay-menu");
  overlayMenu->ignoreAnchorPointForPosition(false);
  overlayMenu->setPosition(widthCS / 2, heightCS / 2);
  overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());
  m_mainLayer->addChild(overlayMenu);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75);

  auto infoBtn = CCMenuItemSpriteExtra::create(
      infoBtnSprite,
      this, menu_selector(ProjectInfoPopupLayer::infoPopup));
  infoBtn->setID("info-button");
  infoBtn->setPosition(m_mainLayer->getScaledContentWidth() - 17.5f, m_mainLayer->getScaledContentHeight() - 17.5f);
  overlayMenu->addChild(infoBtn);

  return true;
};

void ProjectInfoPopupLayer::show()
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