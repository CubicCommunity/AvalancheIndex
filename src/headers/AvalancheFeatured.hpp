#pragma once

#include <Geode/ui/Popup.hpp>

#include <Geode/utils/web.hpp>

#include <Geode/cocos/include/cocos2d.h>

using namespace geode::prelude;

class AvalancheFeatured : public geode::Popup<> {
public:
  static AvalancheFeatured* create();
  void show() override;

protected:
  float m_maxHeight = 220;

  CCMenu* m_overlayMenu;

  CCClippingNode* m_clippingNode;
  CCMenuItemSpriteExtra* m_infoBtn;

  bool setup() override;

  void infoPopup(CCObject*);
  void changelogPopup(CCObject*);
  void openApplicationPopup(CCObject* sender);
};