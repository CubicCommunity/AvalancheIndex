#pragma once

#include "../../incl/Avalanche.hpp"

#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/include/cocos2d.h>

class ProjectInfoPopupLayer : public geode::Popup<>
{
public:
  static ProjectInfoPopupLayer *create();

  ProjectInfoPopupLayer *setProject(avalanche::Project avalProject);

  void show() override;

protected:
  avalanche::Project m_avalProject;

  LoadingCircle *m_loadingCircle = LoadingCircle::create();

  void infoPopup(CCObject *);

  bool setup() override;
};