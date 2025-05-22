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
  EventListener<web::WebTask> m_downloadListener;
  LoadingCircle *m_loadingCircle = LoadingCircle::create();
  CCMenuItemSpriteExtra *m_infoBtn;
  CCClippingNode *m_clippingNode;
  Ref<CCImage> m_image;
  std::string m_viewURL;

  bool setup() override;

  void infoPopup(CCObject *);
  void onDownloadFinished(CCSprite *sprite);
  void onDownloadFail();
  void imageCreationFinished(CCImage *image);
  void openApplicationPopup(CCObject *sender);
};