#pragma once

#include "../../incl/Avalanche.hpp"

#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/include/cocos2d.h>

class ProjectInfoPopup : public geode::Popup<>
{
public:
  static ProjectInfoPopup *create();

  ProjectInfoPopup *setProject(avalanche::Project avalProject);

  void show() override;

protected:
  avalanche::Project m_avalProject;

  void infoPopup(CCObject *);

  bool setup() override;
};