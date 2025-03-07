/*
borrowed from an older version of
https://github.com/cdc-sys/level-thumbs-mod
*/

#pragma once

#ifndef __THUMBIMAGECACHE_H
#define __THUMBIMAGECACHE_H

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ImageCache
{
protected:
    static ImageCache *instance;

public:
    Ref<CCDictionary> m_imageDict;

    ImageCache();
    void addImage(CCImage *image, std::string key);
    CCImage *getImage(std::string key);

    static ImageCache *get()
    {
        if (!instance)
        {
            instance = new ImageCache();
        };

        return instance;
    };
};

#endif