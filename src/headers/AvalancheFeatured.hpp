/*
borrowed from an older version of
https://github.com/cdc-sys/level-thumbs-mod
*/

#pragma once

#include <Geode/ui/Popup.hpp>

#include <Geode/utils/web.hpp>

#include <Geode/cocos/include/cocos2d.h>

using namespace geode::prelude;

class AvalancheFeatured : public geode::Popup<>
{
public:
  static AvalancheFeatured *create();
  void show() override;

protected:
  CCMenu *m_overlayMenu;

  float m_maxHeight = 220;
  CCMenuItemSpriteExtra *m_infoBtn;
  CCClippingNode *m_clippingNode;

  bool setup() override;

  void infoPopup(CCObject *);
  void changelogPopup(CCObject *);
  void openApplicationPopup(CCObject *sender);
};