#include "../ProjectInfoPopup.hpp"

#include <Debugger.hpp>

#include <iomanip>
#include <sstream>

#include <Avalanche.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Notification.hpp>

#include <Geode/utils/web.hpp>

using namespace geode::prelude;
using namespace avalanche;

Handler* avalHandler = Handler::get();

inline std::string url_encode(const std::string& value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (char c : value) {
    if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
    } else {
      escaped << '%' << std::setw(2) << int((unsigned char)c);
    };
  };

  return escaped.str();
};

void ProjectInfoPopup::doInfo(Project proj, std::string publisher) {
  std::ostringstream typeOfProj;

  switch (proj.type) {
  case Project::Type::TEAM:
    typeOfProj << "a <cg>team project</c> hosted by <cy>" << proj.host << "</c>";
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
  body << "<cy>" << publisher << "</c> - '<cg>" << proj.name << "</c>' is " << typeOfProj.str() << ". You can watch its showcase here.";

  std::string resultBody = body.str();

  createQuickPopup(
    proj.name.c_str(),
    resultBody.c_str(),
    "OK", "Watch",
    [proj](auto, bool btn2) {
      if (btn2) {
        AVAL_LOG_INFO("Opening showcase link in browser: {}", proj.showcase);
        web::openLinkInBrowser(proj.showcase);
      } else {
        AVAL_LOG_DEBUG("User clicked OK");
      }; },
    true);
};

void ProjectInfoPopup::doShowcase(Project proj, std::string publisher) {
  std::ostringstream body;
  body << "Watch the full showcase video for <cy>" << publisher << "</c> - '<cg>" << proj.name << "</c>'?";

  std::string resultBody = body.str();

  createQuickPopup(
    proj.name.c_str(),
    resultBody.c_str(),
    "Cancel", "Watch",
    [proj](auto, bool btn2) {
      if (btn2) {
        AVAL_LOG_INFO("Opening showcase link in browser: {}", proj.showcase);
        web::openLinkInBrowser(proj.showcase);
      } else {
        AVAL_LOG_DEBUG("User clicked Cancel");
      }; },
    true);
};

void ProjectInfoPopup::infoPopup(CCObject*) {
  ProjectInfoPopup::doInfo(m_avalProject, m_avalPublisher);
};

void ProjectInfoPopup::onPlayShowcase(CCObject*) {
  ProjectInfoPopup::doShowcase(m_avalProject, m_avalPublisher);
};

void ProjectInfoPopup::infoPopupLinked(CCObject*) {
  ProjectInfoPopup::doInfo(m_linkedProject, m_linkedPublisher);
};

void ProjectInfoPopup::onPlayShowcaseLinked(CCObject*) {
  ProjectInfoPopup::doShowcase(m_linkedProject, m_linkedPublisher);
};

void ProjectInfoPopup::onFameInfo(CCObject*) {
  std::ostringstream body;
  body << "The level '<cg>" << m_avalProject.name << "</c>' by <cy>" << m_avalPublisher << "</c> is featured in <cl>Avalanche's</c> <cy>Hall of Fame</c>. It is a special list of levels that are considered to be the best of the best from the team.";

  std::string resultBody = body.str();

  createQuickPopup(
    "Hall of Fame",
    resultBody.c_str(),
    "OK", "Learn More",
    [](auto, bool btn2) {
      if (btn2) {
        AVAL_LOG_INFO("Opening Hall of Fame link in browser");
        web::openLinkInBrowser(URL_AVALANCHE);
      } else {
        AVAL_LOG_DEBUG("User clicked OK");
      }; },
    true);
};

void ProjectInfoPopup::settingsPopup(CCObject*) {
  AVAL_LOG_INFO("Opening settings popup");
  openSettingsPopup(getMod());
};

bool ProjectInfoPopup::setup() {
  setID("project-popup"_spr);
  setTitle("Loading...");

  m_title->setFntFile("bigFont.fnt");
  m_title->setScale(1.25f);

  auto [widthCS, heightCS] = m_mainLayer->getContentSize();
  auto [widthP, heightP] = m_mainLayer->getPosition();

  // for buttons to work
  m_overlayMenu = CCMenu::create();
  m_overlayMenu->setID("overlay-menu");
  m_overlayMenu->ignoreAnchorPointForPosition(false);
  m_overlayMenu->setPosition({ widthCS / 2.f, heightCS / 2.f });
  m_overlayMenu->setScaledContentSize(m_mainLayer->getScaledContentSize());
  m_overlayMenu->setZOrder(10);

  m_mainLayer->addChild(m_overlayMenu);

  // info button
  auto infoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
  infoBtnSprite->setScale(0.75f);

  auto infoBtn = CCMenuItemSpriteExtra::create(
    infoBtnSprite,
    this,
    menu_selector(ProjectInfoPopup::infoPopup));
  infoBtn->setID("info-button");
  infoBtn->setPosition({ m_mainLayer->getScaledContentWidth() - 15.f, m_mainLayer->getScaledContentHeight() - 15.f });
  infoBtn->setZOrder(126);

  m_overlayMenu->addChild(infoBtn);

  AVAL_LOG_WARN("Project info popup still unfinished, please use ProjectInfoPopup::setProject to finish setting it up before displaying it");

  return true;
};

ProjectInfoPopup* ProjectInfoPopup::setProject(GJGameLevel* level) {
  setZOrder(10);

  m_level = level;
  m_avalProject = avalHandler->GetProject(m_level->m_levelID.value());

  if (m_avalProject.type == Project::Type::NONE) {
    AVAL_LOG_ERROR("Avalanche project type is NONE");
    return this;
  } else if (m_avalProject.type == Project::Type::TEAM) {
    m_avalPublisher = "Avalanche";
  } else {
    m_avalPublisher = m_avalProject.host;
  };

  setTitle(m_avalProject.name);

  // hall of fame deco

  auto fameFrame_layout = AxisLayout::create(Axis::Row);
  fameFrame_layout->setCrossAxisLineAlignment(AxisAlignment::Center);
  fameFrame_layout->setCrossAxisAlignment(AxisAlignment::Center);
  fameFrame_layout->setAxisAlignment(AxisAlignment::Center);
  fameFrame_layout->setGrowCrossAxis(false);
  fameFrame_layout->setAxisReverse(false);
  fameFrame_layout->setAutoScale(false);
  fameFrame_layout->setGap(5.f);

  auto fameFrame = CCMenu::create();
  fameFrame->setID("fame-frame");
  fameFrame->ignoreAnchorPointForPosition(false);
  fameFrame->setPosition({ m_mainLayer->getScaledContentWidth() / 2.f, m_title->getPositionY() - 20.f });
  fameFrame->setScaledContentSize({ m_mainLayer->getScaledContentWidth() * 0.75f, 12.5f });
  fameFrame->setLayout(fameFrame_layout);

  fameFrame->updateLayout(true);
  m_overlayMenu->addChild(fameFrame);

  if (m_avalProject.fame) {
    AVAL_LOG_INFO("Project '{}' is in the Hall of Fame", m_avalProject.name);

    m_title->setFntFile("goldFont.fnt");
    m_title->setScale(1.f);

    m_cornerArtType = "dailyLevelCorner_001.png";

    auto fameIcon = CCSprite::createWithSpriteFrameName("GJ_bigStar_001.png");
    fameIcon->setScale(0.25f);

    auto fameLabel = CCLabelBMFont::create("Avalanche Hall of Fame", "goldFont.fnt");
    fameLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    fameLabel->ignoreAnchorPointForPosition(false);
    fameLabel->setAnchorPoint({ 0.5, 0.5 });
    fameLabel->setScale(0.375f);

    auto fameBtn = CCMenuItemSpriteExtra::create(
      fameLabel,
      this,
      menu_selector(ProjectInfoPopup::onFameInfo));
    fameFrame->addChild(fameIcon);
    fameFrame->addChild(fameBtn);

    fameFrame->updateLayout(true);
  } else {
    AVAL_LOG_DEBUG("Project '{}' is not in the Hall of Fame", m_avalProject.name);
  };

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
  border->setZOrder(0);

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
  m_clippingNode->setZOrder(-1);

  m_mainLayer->addChild(border);
  m_mainLayer->addChild(m_clippingNode);

  auto art_bottomLeft = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_bottomLeft->setID("bottom-left-corner");
  art_bottomLeft->setAnchorPoint({ 0, 0 });
  art_bottomLeft->setPosition({ 0, 0 });
  art_bottomLeft->setScale(1.250f);
  art_bottomLeft->setFlipX(false);
  art_bottomLeft->setFlipY(false);
  art_bottomLeft->setZOrder(0);

  m_overlayMenu->addChild(art_bottomLeft);

  auto art_bottomRight = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_bottomRight->setID("bottom-right-corner");
  art_bottomRight->setAnchorPoint({ 1, 0 });
  art_bottomRight->setPosition({ m_overlayMenu->getScaledContentWidth(), 0 });
  art_bottomRight->setScale(1.250f);
  art_bottomRight->setFlipX(true);
  art_bottomRight->setFlipY(false);
  art_bottomLeft->setZOrder(0);

  m_overlayMenu->addChild(art_bottomRight);

  auto art_topLeft = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_topLeft->setID("top-left-corner");
  art_topLeft->setAnchorPoint({ 0, 1 });
  art_topLeft->setPosition({ 0, m_overlayMenu->getScaledContentHeight() });
  art_topLeft->setScale(1.250f);
  art_topLeft->setFlipX(false);
  art_topLeft->setFlipY(true);
  art_topLeft->setZOrder(0);

  m_overlayMenu->addChild(art_topLeft);

  auto art_topRight = CCSprite::createWithSpriteFrameName(m_cornerArtType.c_str());
  art_topRight->setID("top-right-corner");
  art_topRight->setAnchorPoint({ 1, 1 });
  art_topRight->setPosition({ m_overlayMenu->getScaledContentWidth(), m_overlayMenu->getScaledContentHeight() });
  art_topRight->setScale(1.250f);
  art_topRight->setFlipX(true);
  art_topRight->setFlipY(true);
  art_topRight->setZOrder(0);

  m_overlayMenu->addChild(art_topRight);

  auto hostLabelTxt = "Published by";

  switch (m_avalProject.type) {
  case Project::Type::TEAM:
    hostLabelTxt = "Lead by";
    break;

  case Project::Type::COLLAB:
    hostLabelTxt = "Hosted by";
    break;

  case Project::Type::EVENT:
    hostLabelTxt = "Published by";
    break;

  case Project::Type::SOLO:
    hostLabelTxt = "Created by";
    break;

  default:
    hostLabelTxt = "Published by";
    break;
  };

  auto hostName_label = CCLabelBMFont::create(hostLabelTxt, "bigFont.fnt");
  hostName_label->setID("host-name-label");
  hostName_label->setAnchorPoint({ 0, 1 });
  hostName_label->ignoreAnchorPointForPosition(false);
  hostName_label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
  hostName_label->setPosition({ 10.f, m_mainLayer->getScaledContentHeight() - 55.f });
  hostName_label->setScale(0.25f);

  auto hostName = CCLabelBMFont::create(m_avalProject.host.c_str(), "goldFont.fnt");
  hostName->setID("host-name");
  hostName->setAnchorPoint({ 0, 1 });
  hostName->ignoreAnchorPointForPosition(false);
  hostName->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
  hostName->setPosition({ 10.f, m_mainLayer->getScaledContentHeight() - 65.f });
  hostName->setScale(0.75f);

  m_overlayMenu->addChild(hostName_label);
  m_overlayMenu->addChild(hostName);

  AVAL_LOG_INFO("Adding showcase container to main project '{}'", m_avalProject.name);

  // create showcase project button
  AVAL_LOG_DEBUG("Creating showcase project button");
  auto showcaseProjMenu = CCMenu::create();
  showcaseProjMenu->setID("showcase-project-menu");
  showcaseProjMenu->setAnchorPoint({ 0.5, 0 });
  showcaseProjMenu->ignoreAnchorPointForPosition(false);
  showcaseProjMenu->setPosition({ m_mainLayer->getScaledContentWidth() / 2.f, 12.5f });
  showcaseProjMenu->setScaledContentSize({ 192.f, 108.f });
  showcaseProjMenu->setZOrder(2);

  m_clippingNode->addChild(showcaseProjMenu);

  // create clipping node for showcase project
  AVAL_LOG_DEBUG("Creating clipping node for showcase project");
  auto showcaseProjClippingNode = CCClippingNode::create();
  showcaseProjClippingNode->setID("clipping-node");
  showcaseProjClippingNode->setContentSize(showcaseProjMenu->getScaledContentSize());
  showcaseProjClippingNode->ignoreAnchorPointForPosition(false);
  showcaseProjClippingNode->setAnchorPoint({ 0.5f, 0.5f });
  showcaseProjClippingNode->setPosition({ showcaseProjMenu->getScaledContentWidth() / 2.f, showcaseProjMenu->getScaledContentHeight() / 2.f });
  showcaseProjClippingNode->setStencil(CCLayerColor::create({ 250, 250, 250 }, showcaseProjMenu->getScaledContentWidth(), showcaseProjMenu->getScaledContentHeight()));;
  showcaseProjClippingNode->setZOrder(-1);

  showcaseProjMenu->addChild(showcaseProjClippingNode);

  auto showcaseProjClippingNodeBg = CCScale9Sprite::create("GJ_square04.png");
  showcaseProjClippingNodeBg->setID("background");
  showcaseProjClippingNodeBg->setContentSize(showcaseProjClippingNode->getContentSize());
  showcaseProjClippingNodeBg->setPosition(showcaseProjClippingNode->getPosition());
  showcaseProjClippingNodeBg->setAnchorPoint(showcaseProjClippingNode->getAnchorPoint());
  showcaseProjClippingNodeBg->setZOrder(-1);

  showcaseProjClippingNode->addChild(showcaseProjClippingNodeBg);

  // corner art deco for showcase project
  auto corner = "rewardCorner_001.png";

  auto art_bottomLeft_showcaseProj = CCSprite::createWithSpriteFrameName(corner);
  art_bottomLeft_showcaseProj->setID("bottom-left-corner");
  art_bottomLeft_showcaseProj->setAnchorPoint({ 0, 0 });
  art_bottomLeft_showcaseProj->setPosition({ 0, 0 });
  art_bottomLeft_showcaseProj->setScale(0.75f);
  art_bottomLeft_showcaseProj->setFlipX(false);
  art_bottomLeft_showcaseProj->setFlipY(false);
  art_bottomLeft_showcaseProj->setZOrder(4);

  showcaseProjClippingNode->addChild(art_bottomLeft_showcaseProj);

  auto art_bottomRight_showcaseProj = CCSprite::createWithSpriteFrameName(corner);
  art_bottomRight_showcaseProj->setID("bottom-right-corner");
  art_bottomRight_showcaseProj->setAnchorPoint({ 1, 0 });
  art_bottomRight_showcaseProj->setPosition({ showcaseProjClippingNode->getScaledContentWidth(), 0 });
  art_bottomRight_showcaseProj->setScale(0.75f);
  art_bottomRight_showcaseProj->setFlipX(true);
  art_bottomRight_showcaseProj->setFlipY(false);
  art_bottomRight_showcaseProj->setZOrder(4);

  showcaseProjClippingNode->addChild(art_bottomRight_showcaseProj);

  auto art_topLeft_showcaseProj = CCSprite::createWithSpriteFrameName(corner);
  art_topLeft_showcaseProj->setID("top-left-corner");
  art_topLeft_showcaseProj->setAnchorPoint({ 0, 1 });
  art_topLeft_showcaseProj->setPosition({ 0, showcaseProjClippingNode->getScaledContentHeight() });
  art_topLeft_showcaseProj->setScale(0.75f);
  art_topLeft_showcaseProj->setFlipX(false);
  art_topLeft_showcaseProj->setFlipY(true);
  art_topLeft_showcaseProj->setZOrder(4);

  showcaseProjClippingNode->addChild(art_topLeft_showcaseProj);

  auto art_topRight_showcaseProj = CCSprite::createWithSpriteFrameName(corner);
  art_topRight_showcaseProj->setID("top-right-corner");
  art_topRight_showcaseProj->setAnchorPoint({ 1, 1 });
  art_topRight_showcaseProj->setPosition({ showcaseProjClippingNode->getScaledContentWidth(), showcaseProjClippingNode->getScaledContentHeight() });
  art_topRight_showcaseProj->setScale(0.75f);
  art_topRight_showcaseProj->setFlipX(true);
  art_topRight_showcaseProj->setFlipY(true);
  art_topRight_showcaseProj->setZOrder(4);

  showcaseProjClippingNode->addChild(art_topRight_showcaseProj);

  // create thumbnail lazy sprite for showcase project
  if (AVAL_GEODE_MOD->getSettingValue<bool>("show-proj-thumb")) {
    AVAL_LOG_DEBUG("Creating thumbnail lazy sprite for showcase project '{}'", m_avalProject.name);
    LazySprite* showcaseProjThumb = LazySprite::create(showcaseProjMenu->getScaledContentSize(), true);
    showcaseProjThumb->setID("thumbnail");
    showcaseProjThumb->setAnchorPoint({ 0.5, 0.5 });
    showcaseProjThumb->ignoreAnchorPointForPosition(false);
    showcaseProjThumb->setPosition({ showcaseProjMenu->getScaledContentWidth() / 2.f, showcaseProjMenu->getScaledContentHeight() / 2.f });
    showcaseProjThumb->setScale(0.5f);

    showcaseProjThumb->setLoadCallback([showcaseProjThumb, showcaseProjClippingNode](Result<> res) {
      if (res.isOk()) {
        AVAL_LOG_INFO("Showcase project thumbnail loaded successfully");

        showcaseProjThumb->setScale(1.f);
        showcaseProjThumb->setScale(showcaseProjClippingNode->getScaledContentHeight() / showcaseProjThumb->getScaledContentHeight());

        showcaseProjThumb->setPosition(showcaseProjClippingNode->getPosition());
        showcaseProjThumb->ignoreAnchorPointForPosition(false);
        showcaseProjThumb->setColor({ 125, 125, 125 });
        showcaseProjThumb->setOpacity(125);
      } else {
        AVAL_LOG_ERROR("Failed to load showcase project thumbnail: {}", res.unwrapErr());
        showcaseProjThumb->removeMeAndCleanup();
      };
                                       });

    std::string encodedShowcaseUrl = url_encode(m_avalProject.showcase); // encode the showcase url for use in the thumbnail url
    std::string showcaseProjThumbURL = fmt::format("https://api.cubicstudios.xyz/avalanche/v1/fetch/yt-thumbnails?url={}", (std::string)encodedShowcaseUrl); // custom thumbnail
    if (m_geodeLoader->isModLoaded("prevter.imageplus")) showcaseProjThumbURL.append("?webp");

    AVAL_LOG_DEBUG("Getting showcase project thumbnail at {}...", (std::string)showcaseProjThumbURL);
    showcaseProjThumb->loadFromUrl(showcaseProjThumbURL, LazySprite::Format::kFmtUnKnown, false);
    if (showcaseProjThumb) showcaseProjClippingNode->addChild(showcaseProjThumb);
  } else {
    AVAL_LOG_DEBUG("Showcase project thumbnail setting is disabled, not adding thumbnail to showcase containered project container");
  };

  // set border
  auto showcaseProjBorder = CCScale9Sprite::create("GJ_square07.png");
  showcaseProjBorder->setID("border");
  showcaseProjBorder->setPosition(showcaseProjClippingNode->getPosition());
  showcaseProjBorder->setContentSize(showcaseProjClippingNode->getScaledContentSize());
  showcaseProjBorder->ignoreAnchorPointForPosition(false);
  showcaseProjBorder->setAnchorPoint({ 0.5f, 0.5f });
  showcaseProjBorder->setZOrder(3);

  // add border to clipping node
  showcaseProjClippingNode->addChild(showcaseProjBorder);

  auto playShowcase_label = CCLabelBMFont::create("Watch the\nShowcase", "bigFont.fnt");
  playShowcase_label->setID("play-showcase-label");
  playShowcase_label->setScale(0.5f);
  playShowcase_label->setAnchorPoint({ 0.5, 0.5 });
  playShowcase_label->ignoreAnchorPointForPosition(false);
  playShowcase_label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
  playShowcase_label->setPosition({ showcaseProjMenu->getScaledContentWidth() / 2.f, 75.f });
  playShowcase_label->setZOrder(3);

  auto playShowcase_sprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
  playShowcase_sprite->setScale(0.5f);

  auto playShowcase = CCMenuItemSpriteExtra::create(
    playShowcase_sprite,
    this,
    menu_selector(ProjectInfoPopup::onPlayShowcase));
  playShowcase->setID("play-showcase");
  playShowcase->setPosition({ showcaseProjMenu->getScaledContentWidth() / 2.f, 37.5f });
  playShowcase->setZOrder(4);

  showcaseProjMenu->addChild(playShowcase_label);
  showcaseProjMenu->addChild(playShowcase);

  // project thumbnail
  if (AVAL_GEODE_MOD->getSettingValue<bool>("show-proj-thumb")) {
    AVAL_LOG_DEBUG("Adding project thumbnail to project info popup");

    // create thumbnail sprite
    auto projThumb = LazySprite::create(m_overlayMenu->getScaledContentSize(), true);
    projThumb->setID("thumbnail");
    projThumb->setAnchorPoint({ 0.5, 0.5 });
    projThumb->ignoreAnchorPointForPosition(false);
    projThumb->setPosition({ m_clippingNode->getScaledContentWidth() / 2, m_clippingNode->getScaledContentHeight() / 2 });

    projThumb->setLoadCallback([this, projThumb, bgSize](Result<> res) {
      if (res.isOk()) {
        AVAL_LOG_INFO("Sprite loaded successfully");

        // scale to fit inside bg size
        float scale = bgSize.height / projThumb->getContentHeight();

        projThumb->setScale(scale);
        projThumb->ignoreAnchorPointForPosition(false);
        projThumb->setColor({ 125, 125, 125 });
        projThumb->setOpacity(125);
      } else {
        AVAL_LOG_ERROR("Failed to get project thumbnail, {}", res.unwrapErr());
        projThumb->removeMeAndCleanup();
      };
                               });

    std::string projThumbURL = fmt::format("https://api.cubicstudios.xyz/avalanche/v1/fetch/thumbnails?id={}", m_level->m_levelID.value()); // custom thumbnail
    if (m_geodeLoader->isModLoaded("prevter.imageplus")) projThumbURL.append("?webp");

    AVAL_LOG_DEBUG("Getting thumbnail at {}...", (std::string)projThumbURL);
    projThumb->loadFromUrl(projThumbURL, LazySprite::Format::kFmtUnKnown, false);
    if (projThumb) m_clippingNode->addChild(projThumb);
  } else {
    AVAL_LOG_DEBUG("Project thumbnail setting is disabled, not adding thumbnail to project info popup");
  };

  if (m_avalProject.link_to_main.enabled) {
    AVAL_LOG_DEBUG("Project '{}' has a link to the main project", m_avalProject.name);
    m_linkedProject = avalHandler->GetProject(m_avalProject.link_to_main.level_id);

    if (m_linkedProject.type == Project::Type::NONE) {
      AVAL_LOG_ERROR("Failed to get linked project with ID {}", m_avalProject.link_to_main.level_id);
    } else {
      AVAL_LOG_INFO("Adding link to main project '{}'", m_linkedProject.name);

      // create linked project button
      AVAL_LOG_DEBUG("Creating linked project button");
      auto linkedProjMenu = CCMenu::create();
      linkedProjMenu->setID("linked-project-menu");
      linkedProjMenu->setAnchorPoint({ 1, 1 });
      linkedProjMenu->ignoreAnchorPointForPosition(false);
      linkedProjMenu->setPosition({ m_mainLayer->getScaledContentWidth() - 25.f, m_mainLayer->getScaledContentHeight() - 55.f });
      linkedProjMenu->setScaledContentSize({ 150.f, 81.f });
      linkedProjMenu->setZOrder(2);

      m_clippingNode->addChild(linkedProjMenu);

      // info button
      auto linkedProjInfoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
      linkedProjInfoBtnSprite->setScale(0.375f);

      auto linkedProjInfoBtn = CCMenuItemSpriteExtra::create(
        linkedProjInfoBtnSprite,
        this,
        menu_selector(ProjectInfoPopup::infoPopupLinked));
      linkedProjInfoBtn->setID("info-button");
      linkedProjInfoBtn->setPosition({ linkedProjMenu->getScaledContentWidth() - 6.25f, linkedProjMenu->getScaledContentHeight() - 6.25f });
      linkedProjInfoBtn->setZOrder(126);

      linkedProjMenu->addChild(linkedProjInfoBtn);

      // create clipping node for linked project
      AVAL_LOG_DEBUG("Creating clipping node for linked project");
      auto linkedProjClippingNode = CCClippingNode::create();
      linkedProjClippingNode->setID("clipping-node");
      linkedProjClippingNode->setContentSize(linkedProjMenu->getScaledContentSize());
      linkedProjClippingNode->ignoreAnchorPointForPosition(false);
      linkedProjClippingNode->setAnchorPoint({ 0.5f, 0.5f });
      linkedProjClippingNode->setPosition({ linkedProjMenu->getScaledContentWidth() / 2.f, linkedProjMenu->getScaledContentHeight() / 2.f });
      linkedProjClippingNode->setStencil(CCLayerColor::create({ 250, 250, 250 }, linkedProjMenu->getScaledContentWidth(), linkedProjMenu->getScaledContentHeight()));;
      linkedProjClippingNode->setZOrder(-1);

      linkedProjMenu->addChild(linkedProjClippingNode);

      auto linkedProjClippingNodeBg = CCScale9Sprite::create("GJ_square02.png");
      linkedProjClippingNodeBg->setID("background");
      linkedProjClippingNodeBg->setContentSize(linkedProjClippingNode->getContentSize());
      linkedProjClippingNodeBg->setPosition(linkedProjClippingNode->getPosition());
      linkedProjClippingNodeBg->setAnchorPoint(linkedProjClippingNode->getAnchorPoint());
      linkedProjClippingNodeBg->setZOrder(-1);

      linkedProjClippingNode->addChild(linkedProjClippingNodeBg);

      // corner art deco for linked project
      auto corner = "rewardCorner_001.png";

      auto art_bottomLeft_linkedProj = CCSprite::createWithSpriteFrameName(corner);
      art_bottomLeft_linkedProj->setID("bottom-left-corner");
      art_bottomLeft_linkedProj->setAnchorPoint({ 0, 0 });
      art_bottomLeft_linkedProj->setPosition({ 0, 0 });
      art_bottomLeft_linkedProj->setScale(0.5f);
      art_bottomLeft_linkedProj->setFlipX(false);
      art_bottomLeft_linkedProj->setFlipY(false);
      art_bottomLeft_linkedProj->setZOrder(3);

      linkedProjClippingNode->addChild(art_bottomLeft_linkedProj);

      auto art_bottomRight_linkedProj = CCSprite::createWithSpriteFrameName(corner);
      art_bottomRight_linkedProj->setID("bottom-right-corner");
      art_bottomRight_linkedProj->setAnchorPoint({ 1, 0 });
      art_bottomRight_linkedProj->setPosition({ linkedProjClippingNode->getScaledContentWidth(), 0 });
      art_bottomRight_linkedProj->setScale(0.5f);
      art_bottomRight_linkedProj->setFlipX(true);
      art_bottomRight_linkedProj->setFlipY(false);
      art_bottomRight_linkedProj->setZOrder(3);

      linkedProjClippingNode->addChild(art_bottomRight_linkedProj);

      auto art_topLeft_linkedProj = CCSprite::createWithSpriteFrameName(corner);
      art_topLeft_linkedProj->setID("top-left-corner");
      art_topLeft_linkedProj->setAnchorPoint({ 0, 1 });
      art_topLeft_linkedProj->setPosition({ 0, linkedProjClippingNode->getScaledContentHeight() });
      art_topLeft_linkedProj->setScale(0.5f);
      art_topLeft_linkedProj->setFlipX(false);
      art_topLeft_linkedProj->setFlipY(true);
      art_topLeft_linkedProj->setZOrder(3);

      linkedProjClippingNode->addChild(art_topLeft_linkedProj);

      auto art_topRight_linkedProj = CCSprite::createWithSpriteFrameName(corner);
      art_topRight_linkedProj->setID("top-right-corner");
      art_topRight_linkedProj->setAnchorPoint({ 1, 1 });
      art_topRight_linkedProj->setPosition({ linkedProjClippingNode->getScaledContentWidth(), linkedProjClippingNode->getScaledContentHeight() });
      art_topRight_linkedProj->setScale(0.5f);
      art_topRight_linkedProj->setFlipX(true);
      art_topRight_linkedProj->setFlipY(true);
      art_topRight_linkedProj->setZOrder(3);

      linkedProjClippingNode->addChild(art_topRight_linkedProj);

      // create thumbnail lazy sprite for linked project
      if (AVAL_GEODE_MOD->getSettingValue<bool>("show-proj-thumb")) {
        AVAL_LOG_DEBUG("Creating thumbnail lazy sprite for linked project '{}'", m_linkedProject.name);
        LazySprite* linkedProjThumb = LazySprite::create(linkedProjMenu->getScaledContentSize(), true);
        linkedProjThumb->setID("thumbnail");
        linkedProjThumb->setAnchorPoint({ 0.5, 0.5 });
        linkedProjThumb->ignoreAnchorPointForPosition(false);
        linkedProjThumb->setPosition({ linkedProjMenu->getScaledContentWidth() / 2.f, linkedProjMenu->getScaledContentHeight() / 2.f });
        linkedProjThumb->setScale(0.5f);

        linkedProjThumb->setLoadCallback([linkedProjThumb, linkedProjClippingNode](Result<> res) {
          if (res.isOk()) {
            AVAL_LOG_INFO("Linked project thumbnail loaded successfully");

            linkedProjThumb->setScale(1.f);
            linkedProjThumb->setScale(linkedProjClippingNode->getScaledContentHeight() / linkedProjThumb->getScaledContentHeight());

            linkedProjThumb->setPosition(linkedProjClippingNode->getPosition());
            linkedProjThumb->ignoreAnchorPointForPosition(false);
            linkedProjThumb->setColor({ 175, 175, 175 });
            linkedProjThumb->setOpacity(175);
          } else {
            AVAL_LOG_ERROR("Failed to load linked project thumbnail: {}", res.unwrapErr());
            linkedProjThumb->removeMeAndCleanup();
          };
                                         });

        std::string encodedShowcaseUrl = url_encode(m_linkedProject.showcase); // encode the showcase url for use in the thumbnail url
        std::string linkedProjThumbURL = fmt::format("https://api.cubicstudios.xyz/avalanche/v1/fetch/yt-thumbnails?url={}", (std::string)encodedShowcaseUrl); // custom thumbnail
        if (m_geodeLoader->isModLoaded("prevter.imageplus")) linkedProjThumbURL.append("?webp");

        AVAL_LOG_DEBUG("Getting linked project thumbnail at {}...", (std::string)linkedProjThumbURL);
        linkedProjThumb->loadFromUrl(linkedProjThumbURL, LazySprite::Format::kFmtUnKnown, false);
        if (linkedProjThumb) linkedProjClippingNode->addChild(linkedProjThumb);
      } else {
        AVAL_LOG_DEBUG("Linked project thumbnail setting is disabled, not adding thumbnail to linked project container");
      };

      // set border
      auto linkedProjBorder = CCScale9Sprite::create("GJ_square07.png");
      linkedProjBorder->setID("border");
      linkedProjBorder->setPosition(linkedProjClippingNode->getPosition());
      linkedProjBorder->setContentSize(linkedProjClippingNode->getScaledContentSize());
      linkedProjBorder->ignoreAnchorPointForPosition(false);
      linkedProjBorder->setAnchorPoint({ 0.5f, 0.5f });
      linkedProjBorder->setZOrder(2);

      // add border to clipping node
      linkedProjClippingNode->addChild(linkedProjBorder);

      // create sprite for linked project showcase button
      auto linkedProjShowcase_sprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
      linkedProjShowcase_sprite->setScale(0.375f);

      // create button to play linked project showcase
      AVAL_LOG_DEBUG("Creating button to play linked project showcase");
      auto linkedProjShowcase = CCMenuItemSpriteExtra::create(
        linkedProjShowcase_sprite,
        this,
        menu_selector(ProjectInfoPopup::onPlayShowcaseLinked));
      linkedProjShowcase->setID("button");
      linkedProjShowcase->setPosition({ linkedProjMenu->getScaledContentWidth() / 2.f, 25.f });

      linkedProjMenu->addChild(linkedProjShowcase);

      // create label for linked project showcase
      AVAL_LOG_DEBUG("Creating label for linked project showcase");
      auto linkedProjLabel = CCLabelBMFont::create("Play Now!", "bigFont.fnt");
      linkedProjLabel->setID("label");
      linkedProjLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
      linkedProjLabel->setPosition({ linkedProjMenu->getScaledContentWidth() / 2.f, linkedProjMenu->getScaledContentHeight() - 12.5f });
      linkedProjLabel->setScale(0.25f);

      linkedProjMenu->addChild(linkedProjLabel);

      // create name text label for linked project name
      AVAL_LOG_DEBUG("Creating label for linked project name");
      auto linkedProjName = CCLabelBMFont::create(m_linkedProject.name.c_str(), "goldFont.fnt");
      linkedProjName->setID("name");
      linkedProjName->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
      linkedProjName->setPosition({ linkedProjMenu->getScaledContentWidth() / 2.f, linkedProjMenu->getScaledContentHeight() - 25.f });
      linkedProjName->setScale(0.625f);

      linkedProjMenu->addChild(linkedProjName);
    };
  } else {
    AVAL_LOG_DEBUG("Project '{}' does not have a link to the main project", m_avalProject.name);
  };

  // geode settings popup button
  auto settingsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
  settingsBtnSprite->setScale(0.75f);

  auto settingsBtn = CCMenuItemSpriteExtra::create(
    settingsBtnSprite,
    this,
    menu_selector(ProjectInfoPopup::settingsPopup));
  settingsBtn->setID("settings-button");
  settingsBtn->setPosition({ 25, 25 });

  m_overlayMenu->addChild(settingsBtn);

  // TODO: add more info

  return this;
};

ProjectInfoPopup* ProjectInfoPopup::create() {
  auto ret = new ProjectInfoPopup();

  if (ret && ret->initAnchored(440, 290)) {
    ret->autorelease();
    return ret;
  };

  CC_SAFE_DELETE(ret);
  return nullptr;
};