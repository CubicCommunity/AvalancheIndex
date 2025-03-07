/*
borrowed from an older version of
https://github.com/cdc-sys/level-thumbs-mod
*/

#include "../ImageCache.hpp"

#include <Geode/Geode.hpp>

ImageCache *ImageCache::instance = nullptr;

ImageCache::ImageCache()
{
    m_imageDict = CCDictionary::create();
};

void ImageCache::addImage(CCImage *image, std::string key)
{
    if (image)
    {
        if (m_imageDict->count() >= 1)
        {
            m_imageDict->removeObjectForKey(static_cast<CCString *>(m_imageDict->allKeys()->objectAtIndex(0))->getCString());
        };

        m_imageDict->setObject(image, key);
    };
};

CCImage *ImageCache::getImage(std::string key)
{
    return static_cast<CCImage *>(m_imageDict->objectForKey(key));
};