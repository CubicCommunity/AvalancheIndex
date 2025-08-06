#include "../AvalancheFeatured.hpp"

#include <Debugger.hpp>

#include <sstream>

#include <Geode/Geode.hpp>

#include <Geode/ui/General.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/LazySprite.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/terminate.hpp>

using namespace geode::prelude;

void AvalancheFeatured::infoPopup(CCObject*) {
  AVAL_LOG_INFO("Opening featured info popup");
  createQuickPopup(
    "Avalanche Featured",
    "This is the latest project <cl>Avalanche</c> is currently working on. If the form is open, you can <cg>apply to join the team</c> to work on this and future projects.",
    "OK", "Apply",
    [](auto, bool btn2) {
      if (btn2) {
        web::openLinkInBrowser("https://gh.cubicstudios.xyz/WebLPS/apply/");
      } else {
        AVAL_LOG_DEBUG("User clicked OK");
      }; },
    true);
};

void AvalancheFeatured::changelogPopup(CCObject*) {
  AVAL_LOG_INFO("Opening changelog popup");
  openChangelogPopup(AVAL_GEODE_MOD);
};

void AvalancheFeatured::openApplicationPopup(CCObject*) {
  AVAL_LOG_INFO("Opening team application popup");
  createQuickPopup(
    "Learn More",
    "Would you like to check out the latest <cl>Avalanche</c> project?",
    "Cancel", "Yes",
    [this](auto, bool btn2) {
      if (btn2) {
        web::openLinkInBrowser("https://gh.cubicstudios.xyz/WebLPS/aval-project/");
      } else {
        AVAL_LOG_DEBUG("User clicked Cancel");
      }; },
    true);
};

bool AvalancheFeatured::setup() {
  m_noElasticity = false;

  setID("featured-popup"_spr);
  setTitle("");

  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();

  auto buttons_height = 0.82f * heightCS;

  // for buttons to work
  m_overlayMenu = CCMenu::create();
  m_overlayMenu->setID("overlay-menu");
  m_overlayMenu->ignoreAnchorPointForPosition(false);
  m_overlayMenu->setPosition({ widthCS / 2.f, heightCS / 2.f });
  m_overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());
  m_overlayMenu->setZOrder(10);

  m_mainLayer->addChild(m_overlayMenu);

  // featured header sprite
  auto titleSprite = CCSprite::createWithSpriteFrameName("featured.png"_spr);
  titleSprite->setID("title-sprite");
  titleSprite->ignoreAnchorPointForPosition(false);
  titleSprite->setPosition({ widthCS / 2, heightCS - 2.5f });

  m_overlayMenu->addChild(titleSprite);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75);

  auto infoBtn = CCMenuItemSpriteExtra::create(
    infoBtnSprite,
    this,
    menu_selector(AvalancheFeatured::infoPopup)
  );
  infoBtn->setID("info-button");
  infoBtn->setPosition({ m_mainLayer->getScaledContentWidth() - 15.f, m_mainLayer->getScaledContentHeight() - 15.f });
  infoBtn->setZOrder(126);

  m_overlayMenu->addChild(infoBtn);

  // corner art deco

  auto art_bottomLeft = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_bottomLeft->setID("bottom-left-corner");
  art_bottomLeft->setAnchorPoint({ 0, 0 });
  art_bottomLeft->setPosition({ 0, 0 });
  art_bottomLeft->setScale(1.250f);
  art_bottomLeft->setFlipX(false);
  art_bottomLeft->setFlipY(false);
  art_bottomLeft->setZOrder(0);

  m_overlayMenu->addChild(art_bottomLeft);

  auto art_bottomRight = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_bottomRight->setID("bottom-right-corner");
  art_bottomRight->setAnchorPoint({ 1, 0 });
  art_bottomRight->setPosition({ m_overlayMenu->getScaledContentWidth(), 0 });
  art_bottomRight->setScale(1.250f);
  art_bottomRight->setFlipX(true);
  art_bottomRight->setFlipY(false);
  art_bottomLeft->setZOrder(0);

  m_overlayMenu->addChild(art_bottomRight);

  auto art_topLeft = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_topLeft->setID("top-left-corner");
  art_topLeft->setAnchorPoint({ 0, 1 });
  art_topLeft->setPosition({ 0, m_overlayMenu->getScaledContentHeight() });
  art_topLeft->setScale(1.250f);
  art_topLeft->setFlipX(false);
  art_topLeft->setFlipY(true);
  art_topLeft->setZOrder(0);

  m_overlayMenu->addChild(art_topLeft);

  auto art_topRight = CCSprite::createWithSpriteFrameName("rewardCorner_001.png");
  art_topRight->setID("top-right-corner");
  art_topRight->setAnchorPoint({ 1, 1 });
  art_topRight->setPosition({ m_overlayMenu->getScaledContentWidth(), m_overlayMenu->getScaledContentHeight() });
  art_topRight->setScale(1.250f);
  art_topRight->setFlipX(true);
  art_topRight->setFlipY(true);
  art_topRight->setZOrder(0);

  m_overlayMenu->addChild(art_topRight);

  // for popup
  m_bgSprite->setZOrder(-2);

  auto bgSize = m_bgSprite->getContentSize();
  auto bgCenter = CCPoint(bgSize.width / 2.f, bgSize.height / 2.f);

  // set border
  auto border = CCScale9Sprite::create("GJ_square07.png");
  border->setID("border");
  border->setContentSize(bgSize);
  border->ignoreAnchorPointForPosition(false);
  border->setAnchorPoint({ 0.5f, 0.5f });
  border->setPosition(bgCenter);
  border->setZOrder(1);

  // create mask
  auto mask = CCLayerColor::create({ 255, 255, 255 });
  mask->setContentSize(bgSize);
  mask->ignoreAnchorPointForPosition(false);
  mask->setAnchorPoint({ 0.5f, 0.5f });
  mask->setPosition(bgCenter);

  // add clipping node
  m_clippingNode = CCClippingNode::create();
  m_clippingNode->setID("clipping-node");
  m_clippingNode->setContentSize(bgSize);
  m_clippingNode->ignoreAnchorPointForPosition(false);
  m_clippingNode->setAnchorPoint({ 0.5f, 0.5f });
  m_clippingNode->setPosition(bgCenter);
  m_clippingNode->setStencil(mask);
  m_clippingNode->setZOrder(0);

  m_mainLayer->addChild(border);
  m_mainLayer->addChild(m_clippingNode);

  // links to project video
  ButtonSprite* infoSprite = ButtonSprite::create("View");
  m_infoBtn = CCMenuItemSpriteExtra::create(
    infoSprite,
    this,
    menu_selector(AvalancheFeatured::openApplicationPopup)
  );
  m_infoBtn->setPosition({ widthCS / 2.f, 2.5f });
  m_infoBtn->setVisible(true);
  m_infoBtn->setZOrder(3);

  m_buttonMenu->addChild(m_infoBtn);

  // featured project thumbnail
  auto projThumb = LazySprite::create(m_overlayMenu->getScaledContentSize(), true);
  projThumb->setID("thumbnail");
  projThumb->setPosition({ m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f });

  projThumb->setLoadCallback([this, projThumb](Result<> res) {
    if (res.isOk()) {
      // Success: scale and position the sprite
      AVAL_LOG_INFO("Sprite loaded successfully");
    } else {
      // Failure: set fallback image
      AVAL_LOG_ERROR("Sprite failed to load, setting fallback: {}", res.unwrapErr());
      projThumb->initWithSpriteFrameName("unavailable.png"_spr);
    };

    float scale = m_maxHeight / projThumb->getScaledContentHeight();

    projThumb->setScale(scale);
                             });

  std::string imgUrl = "https://api.cubicstudios.xyz/avalanche/v1/featured/thumbnail";
  if (Loader::get()->isModLoaded("prevter.imageplus")) imgUrl.append("?webp");

  projThumb->loadFromUrl(imgUrl, LazySprite::Format::kFmtUnKnown, false);
  m_clippingNode->addChild(projThumb);

  if (AVAL_GEODE_MOD->getSettingValue<bool>("dev-mode")) {
    AVAL_LOG_INFO("Dev buttons are enabled");

    // geode loader
    auto loader = Loader::get();

    // geode changelog popup button
    auto changelogBtnSprite = CCSprite::createWithSpriteFrameName("GJ_menuBtn_001.png");
    changelogBtnSprite->setScale(0.625f);

    auto changelogBtn = CCMenuItemSpriteExtra::create(
      changelogBtnSprite,
      this,
      menu_selector(AvalancheFeatured::changelogPopup)
    );
    changelogBtn->setID("changelog-button");
    changelogBtn->setPosition({ 25.f, 25.f });

    m_overlayMenu->addChild(changelogBtn);

    // mod version text label
    std::ostringstream verLabelText;
    verLabelText << AVAL_GEODE_MOD->getName() << " mod " << AVAL_GEODE_MOD->getVersion().toVString(true);

    auto verLabelTextStr = verLabelText.str();

    auto verLabel = CCLabelBMFont::create(verLabelTextStr.c_str(), "bigFont.fnt");
    verLabel->setID("version-label");
    verLabel->ignoreAnchorPointForPosition(false);
    verLabel->setPosition({ m_overlayMenu->getScaledContentWidth() - 5.f, 5.f });
    verLabel->setAnchorPoint({ 1, 0 });
    verLabel->setOpacity(100);
    verLabel->setScale(0.25f);
    verLabel->setZOrder(3);

    m_overlayMenu->addChild(verLabel);

    // game and geode version text label
    std::ostringstream gdVerLabelText;
    gdVerLabelText << "Geode " << loader->getVersion().toVString(true) << " for v" << loader->getGameVersion();

    auto gdVerLabelTextStr = gdVerLabelText.str();

    auto gdVerLabel = CCLabelBMFont::create(gdVerLabelTextStr.c_str(), "bigFont.fnt");
    gdVerLabel->setID("gd-version-label");
    gdVerLabel->ignoreAnchorPointForPosition(false);
    gdVerLabel->setPosition({ m_overlayMenu->getScaledContentWidth() - 5.f, 12.5f });
    gdVerLabel->setAnchorPoint({ 1, 0 });
    gdVerLabel->setOpacity(100);
    gdVerLabel->setScale(0.25f);
    gdVerLabel->setZOrder(3);

    m_overlayMenu->addChild(gdVerLabel);
  } else {
    AVAL_LOG_DEBUG("Dev buttons are disabled");
  };

  return true;
};

AvalancheFeatured* AvalancheFeatured::create() {
  auto ret = new AvalancheFeatured();

  if (ret && ret->initAnchored(395, 225)) {
    ret->autorelease();
    return ret;
  };

  CC_SAFE_DELETE(ret);
  return nullptr;
};