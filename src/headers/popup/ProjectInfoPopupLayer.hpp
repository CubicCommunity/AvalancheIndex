#pragma once

#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/include/cocos2d.h>

class ProjectInfoPopupLayer : public geode::Popup<>
{
public:
  static ProjectInfoPopupLayer *create();
  void show() override;

protected:
  bool setup() override;

  void infoPopup(CCObject *);
};