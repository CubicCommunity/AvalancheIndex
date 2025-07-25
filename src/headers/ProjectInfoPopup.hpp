#pragma once

#include "../../incl/Avalanche.hpp"

#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/include/cocos2d.h>

using namespace avalanche;
using namespace geode::prelude;

class ProjectInfoPopup : public geode::Popup<> {
public:
  static ProjectInfoPopup* create();

  ProjectInfoPopup* setProject(GJGameLevel* level);

  void show() override;
protected:
  std::string m_avalPublisher = "Avalanche";
  std::string m_linkedPublisher = "Avalanche";
  std::string m_cornerArtType = "rewardCorner_001.png";

  Project m_avalProject;
  Project m_linkedProject;

  GJGameLevel* m_level;

  CCMenu* m_overlayMenu;

  CCClippingNode* m_clippingNode;

  void doInfo(Project proj, std::string publisher);
  void doShowcase(Project proj, std::string publisher);

  void infoPopup(CCObject*);
  void settingsPopup(CCObject*);

  void onFameInfo(CCObject*);

  void onPlayShowcase(CCObject*);

  void infoPopupLinked(CCObject*);
  void onPlayShowcaseLinked(CCObject*);

  bool setup() override;
};