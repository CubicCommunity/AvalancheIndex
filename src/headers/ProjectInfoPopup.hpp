#pragma once

#include <Avalanche.hpp>

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace avalanche;

class ProjectInfoPopup : public Popup<> {
public:
  static ProjectInfoPopup* create();

  ProjectInfoPopup* setProject(GJGameLevel* level);
protected:
  std::string m_avalPublisher = "Avalanche";
  std::string m_linkedPublisher = "Avalanche";
  std::string m_cornerArtType = "rewardCorner_001.png";

  Loader* m_geodeLoader = Loader::get();

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