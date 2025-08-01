#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AvalancheFeatured : public Popup<> {
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