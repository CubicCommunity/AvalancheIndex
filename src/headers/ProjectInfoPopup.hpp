#pragma once

#include "../../incl/Avalanche.hpp"

#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/include/cocos2d.h>

using namespace avalanche;
using namespace geode::prelude;

class ProjectInfoPopup : public geode::Popup<>
{
public:
  static ProjectInfoPopup *create();

  ProjectInfoPopup *setProject(GJGameLevel *level);

  void show() override;

protected:
  GJGameLevel *m_level;
  Project m_avalProject;
  std::string m_avalPublisher = "Avalanche";

  CCMenu *m_overlayMenu;

  std::string m_cornerArtType = "rewardCorner_001.png";

  void infoPopup(CCObject *);
  void onFameInfo(CCObject *);

  void onPlayShowcase(CCObject *);

  bool setup() override;
};