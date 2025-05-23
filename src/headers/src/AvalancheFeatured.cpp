/*
borrowed from an older version of
https://github.com/cdc-sys/level-thumbs-mod
*/

#include "../AvalancheFeatured.hpp"
#include "../ImageCache.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/cocos.hpp>

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

AvalancheFeatured *AvalancheFeatured::create()
{
  auto ret = new AvalancheFeatured();

  if (ret && ret->initAnchored(395, 225, "GJ_square05.png"))
  {
    ret->autorelease();
    return ret;
  };

  CC_SAFE_DELETE(ret);
  return nullptr;
};

void AvalancheFeatured::infoPopup(CCObject *)
{
  geode::createQuickPopup(
      "Avalanche Featured",
      "This is the latest project <cl>Avalanche</c> is currently working on. If the form is open, you can <cg>apply to join the team</c> to work on this and future projects.",
      "OK", "Apply",
      [](auto, bool btn2)
      {
        if (btn2)
        {
          web::openLinkInBrowser("https://gh.cubicstudios.xyz/WebLPS/apply/");
        };
      });
};

void AvalancheFeatured::openApplicationPopup(CCObject *)
{
  createQuickPopup(
      "Learn More",
      "Would you like to check out the latest <cl>Avalanche</c> project?",
      "Cancel", "Yes",
      [this](auto, bool btn2)
      {
        if (btn2)
        {
          CCApplication::sharedApplication()->openURL("https://gh.cubicstudios.xyz/WebLPS/aval-project/");
        };
      });
};

bool AvalancheFeatured::setup()
{
  m_noElasticity = false;

  setID("featured-popup"_spr);
  setTitle("");

  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();

  const auto buttons_height = 0.82f * heightCS;

  // for buttons to work
  m_overlayMenu = CCMenu::create();
  m_overlayMenu->setID("overlay-menu");
  m_overlayMenu->ignoreAnchorPointForPosition(false);
  m_overlayMenu->setPosition(widthCS / 2, heightCS / 2);
  m_overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());
  m_overlayMenu->setZOrder(10);
  m_mainLayer->addChild(m_overlayMenu);

  auto titleSprite = CCSprite::create("featured.png"_spr);
  titleSprite->setID("title-sprite");
  titleSprite->ignoreAnchorPointForPosition(false);
  titleSprite->setPosition({widthCS / 2, heightCS - 2.5f});

  m_overlayMenu->addChild(titleSprite);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75);

  auto infoBtn = CCMenuItemSpriteExtra::create(
      infoBtnSprite,
      this, menu_selector(AvalancheFeatured::infoPopup));
  infoBtn->setID("info-button");
  infoBtn->setPosition(m_mainLayer->getScaledContentWidth() - 15.f, m_mainLayer->getScaledContentHeight() - 15.f);
  infoBtn->setZOrder(126);

  m_overlayMenu->addChild(infoBtn);

  // corner art deco

  auto art_bottomLeft = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_bottomLeft->setID("bottom-left-corner");
  art_bottomLeft->setAnchorPoint({0, 0});
  art_bottomLeft->setPosition({0, 0});
  art_bottomLeft->setScale(1.250);
  art_bottomLeft->setFlipX(false);
  art_bottomLeft->setFlipY(false);
  art_bottomLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_bottomLeft);

  auto art_bottomRight = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_bottomRight->setID("bottom-right-corner");
  art_bottomRight->setAnchorPoint({1, 0});
  art_bottomRight->setPosition({m_overlayMenu->getScaledContentWidth(), 0});
  art_bottomRight->setScale(1.250);
  art_bottomRight->setFlipX(true);
  art_bottomRight->setFlipY(false);
  art_bottomLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_bottomRight);

  auto art_topLeft = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_topLeft->setID("top-left-corner");
  art_topLeft->setAnchorPoint({0, 1});
  art_topLeft->setPosition({0, m_overlayMenu->getScaledContentHeight()});
  art_topLeft->setScale(1.250);
  art_topLeft->setFlipX(false);
  art_topLeft->setFlipY(true);
  art_topLeft->setZOrder(-1);

  m_overlayMenu->addChild(art_topLeft);

  auto art_topRight = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_topRight->setID("top-right-corner");
  art_topRight->setAnchorPoint({1, 1});
  art_topRight->setPosition({m_overlayMenu->getScaledContentWidth(), m_overlayMenu->getScaledContentHeight()});
  art_topRight->setScale(1.250);
  art_topRight->setFlipX(true);
  art_topRight->setFlipY(true);
  art_topRight->setZOrder(-1);

  m_overlayMenu->addChild(art_topRight);

  CCScale9Sprite *border = CCScale9Sprite::create("GJ_square07.png");
  border->setContentSize(m_bgSprite->getContentSize());
  border->setPosition(m_bgSprite->getPosition());
  border->setZOrder(3);

  CCLayerColor *mask = CCLayerColor::create({255, 255, 255});
  mask->setContentSize({500.f, 281.f});
  mask->setPosition({m_bgSprite->getContentSize().width / 2 - 391 / 2, m_bgSprite->getContentSize().height / 2 - 220 / 2});

  m_bgSprite->setColor({50, 50, 50});

  m_clippingNode = CCClippingNode::create();
  m_clippingNode->setContentSize(m_bgSprite->getContentSize());
  m_clippingNode->setStencil(mask);
  m_clippingNode->setZOrder(1);

  m_mainLayer->addChild(border);
  m_mainLayer->addChild(m_clippingNode);

  ButtonSprite *infoSprite = ButtonSprite::create("View");
  m_infoBtn = CCMenuItemSpriteExtra::create(infoSprite, this, menu_selector(AvalancheFeatured::openApplicationPopup));

  m_infoBtn->setPosition({widthCS / 2, 6});
  m_infoBtn->setVisible(true);
  m_infoBtn->setZOrder(3);

  m_buttonMenu->addChild(m_infoBtn);

  m_loadingCircle->setParentLayer(m_mainLayer);
  m_loadingCircle->setPosition({widthCS / 2, heightCS / 2});
  m_loadingCircle->ignoreAnchorPointForPosition(false);
  m_loadingCircle->setAnchorPoint({0.5, 0.5});
  m_loadingCircle->setScale(1.f);
  m_loadingCircle->show();

  if (CCImage *image = ImageCache::get()->getImage("projectThumbnail"))
  {
    m_image = image;
    m_loadingCircle->fadeAndRemove();
    imageCreationFinished(m_image);
    return true;
  }
  else
  {
    auto reqThumb = web::WebRequest();

    m_downloadListener.bind([this](web::WebTask::Event *e)
                            {
        if (auto res = e->getValue())
        {
            if (!res->ok()) {
                onDownloadFail();
            } else {
                auto data = res->data();

                std::thread imageThread = std::thread([data,this](){
                    m_image = new CCImage();
                    m_image->autorelease();
                    m_image->initWithImageData(const_cast<uint8_t*>(data.data()),data.size());

                    geode::Loader::get()->queueInMainThread([this](){
                        ImageCache::get()->addImage(m_image, "projectThumbnail");
                        imageCreationFinished(m_image);
                    });
                });

                imageThread.detach();
            };
        }; });

    auto downloadTask = reqThumb.get("https://raw.githubusercontent.com/CubicCommunity/WebLPS/main/aval-project/thumbnail.png");
    m_downloadListener.setFilter(downloadTask);
  };

  return true;
};

void AvalancheFeatured::imageCreationFinished(CCImage *image)
{
  CCTexture2D *texture = new CCTexture2D();
  texture->autorelease();
  texture->initWithImage(image);
  onDownloadFinished(CCSprite::createWithTexture(texture));
};

void AvalancheFeatured::onDownloadFinished(CCSprite *image)
{
  float scale = m_maxHeight / image->getContentSize().height;

  image->setScale(scale);
  image->setUserObject("scale", CCFloat::create(scale));
  image->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() / 2});

  image->setID("thumbnail");
  m_clippingNode->addChild(image);
  m_loadingCircle->fadeAndRemove();
};

void AvalancheFeatured::onDownloadFail()
{
  CCSprite *image = CCSprite::create("unavailable.png"_spr);
  float scale = m_maxHeight / image->getContentSize().height;

  image->setScale(scale);
  image->setUserObject("scale", CCFloat::create(scale));
  image->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() / 2});
  image->setID("thumbnail");

  m_clippingNode->addChild(image);
  m_loadingCircle->fadeAndRemove();
};

void AvalancheFeatured::show()
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