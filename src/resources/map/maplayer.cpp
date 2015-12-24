/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2015  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "resources/map/maplayer.h"

#include "configuration.h"

#ifndef USE_OPENGL
#include "render/graphics.h"
#endif

#include "being/localplayer.h"

#include "enums/resources/map/blockmask.h"
#include "enums/resources/map/mapitemtype.h"

#include "gui/userpalette.h"

#include "render/graphics.h"

#include "resources/image.h"

#include "resources/map/mapitem.h"
#include "resources/map/maprowvertexes.h"
#include "resources/map/metatile.h"
#include "resources/map/speciallayer.h"

#include "debug.h"

MapLayer::MapLayer(const int x,
                   const int y,
                   const int width,
                   const int height,
                   const bool fringeLayer,
                   const int mask,
                   const int tileCondition) :
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height),
    mTiles(new TileInfo[mWidth * mHeight]),
    mDrawLayerFlags(MapType::NORMAL),
    mSpecialLayer(nullptr),
    mTempLayer(nullptr),
    mTempRows(),
    mMask(mask),
    mTileCondition(tileCondition),
    mActorsFix(0),
    mIsFringeLayer(fringeLayer),
    mHighlightAttackRange(config.getBoolValue("highlightAttackRange")),
    mSpecialFlag(true)
{
//    std::fill_n(mTiles, mWidth * mHeight, static_cast<Image*>(nullptr));

    config.addListener("highlightAttackRange", this);
}

MapLayer::~MapLayer()
{
    config.removeListener("highlightAttackRange", this);
    CHECKLISTENERS
    delete []mTiles;
    delete_all(mTempRows);
    mTempRows.clear();
}

void MapLayer::optionChanged(const std::string &value) restrict
{
    if (value == "highlightAttackRange")
    {
        mHighlightAttackRange =
            config.getBoolValue("highlightAttackRange");
    }
}

void MapLayer::setTile(const int x,
                       const int y,
                       Image *const img) restrict
{
    mTiles[x + y * mWidth].image = img;
}

void MapLayer::draw(Graphics *const graphics,
                    int startX,
                    int startY,
                    int endX,
                    int endY,
                    const int scrollX,
                    const int scrollY) const restrict
{
    if (!localPlayer)
        return;

    BLOCK_START("MapLayer::draw")
    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0)
        startX = 0;
    if (startY < 0)
        startY = 0;
    if (endX > mWidth)
        endX = mWidth;
    if (endY > mHeight)
        endY = mHeight;

    const int dx = (mX * mapTileSize) - scrollX;
    const int dy = (mY * mapTileSize) - scrollY + mapTileSize;

    for (int y = startY; y < endY; y++)
    {
        const int y32 = y * mapTileSize;
        const int yWidth = y * mWidth;

        const int py0 = y32 + dy;

        TileInfo *tilePtr = &mTiles[static_cast<size_t>(startX + yWidth)];

        for (int x = startX; x < endX; x++, tilePtr++)
        {
            if (!tilePtr->isEnabled)
                continue;
            const int x32 = x * mapTileSize;

            int c = 0;
            const Image *const img = tilePtr->image;
            if (img)
            {
                const int px = x32 + dx;
                const int py = py0 - img->mBounds.h;
                if (mSpecialFlag ||
                    img->mBounds.h <= mapTileSize)
                {
                    int width = 0;
                    // here need not draw over player position
                    c = getTileDrawWidth(tilePtr, endX - x, width);

                    if (!c)
                    {
                        graphics->drawImage(img, px, py);
                    }
                    else
                    {
                        graphics->drawPattern(img, px, py,
                            width, img->mBounds.h);
                    }
                }
            }

            x += c;
            tilePtr += c;
        }
    }
    BLOCK_END("MapLayer::draw")
}

void MapLayer::drawSDL(Graphics *const graphics) const restrict2
{
    BLOCK_START("MapLayer::drawSDL")
    MapRows::const_iterator rit = mTempRows.begin();
    const MapRows::const_iterator rit_end = mTempRows.end();
    while (rit != rit_end)
    {
        MapRowImages *const images = &(*rit)->images;
        MapRowImages::const_iterator iit = images->begin();
        const MapRowImages::const_iterator iit_end = images->end();
        while (iit != iit_end)
        {
            graphics->drawTileVertexes(*iit);
            ++ iit;
        }
        ++ rit;
    }
    BLOCK_END("MapLayer::drawSDL")
}

#ifdef USE_OPENGL
void MapLayer::updateSDL(const Graphics *const graphics,
                         int startX,
                         int startY,
                         int endX,
                         int endY,
                         const int scrollX,
                         const int scrollY) restrict2
{
    BLOCK_START("MapLayer::updateSDL")
    delete_all(mTempRows);
    mTempRows.clear();

    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0)
        startX = 0;
    if (startY < 0)
        startY = 0;
    if (endX > mWidth)
        endX = mWidth;
    if (endY > mHeight)
        endY = mHeight;

    const int dx = (mX * mapTileSize) - scrollX;
    const int dy = (mY * mapTileSize) - scrollY + mapTileSize;

    for (int y = startY; y < endY; y++)
    {
        MapRowVertexes *const row = new MapRowVertexes();
        mTempRows.push_back(row);

        const Image *lastImage = nullptr;
        ImageVertexes *imgVert = nullptr;

        const int yWidth = y * mWidth;
        const int py0 = y * mapTileSize + dy;
        TileInfo *tilePtr = &mTiles[static_cast<size_t>(startX + yWidth)];

        for (int x = startX; x < endX; x++, tilePtr++)
        {
            if (!tilePtr->isEnabled)
                continue;
            Image *const img = (*tilePtr).image;
            if (img)
            {
                const int px = x * mapTileSize + dx;
                const int py = py0 - img->mBounds.h;
                if (mSpecialFlag ||
                    img->mBounds.h <= mapTileSize)
                {
                    if (lastImage != img)
                    {
                        imgVert = new ImageVertexes();
                        imgVert->image = img;
                        row->images.push_back(imgVert);
                        lastImage = img;
                    }
                    graphics->calcTileSDL(imgVert, px, py);
                }
            }
        }
    }
    BLOCK_END("MapLayer::updateSDL")
}

void MapLayer::updateOGL(Graphics *const graphics,
                         int startX,
                         int startY,
                         int endX,
                         int endY,
                         const int scrollX,
                         const int scrollY) restrict2
{
    BLOCK_START("MapLayer::updateOGL")
    delete_all(mTempRows);
    mTempRows.clear();

    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0)
        startX = 0;
    if (startY < 0)
        startY = 0;
    if (endX > mWidth)
        endX = mWidth;
    if (endY > mHeight)
        endY = mHeight;

    const int dx = (mX * mapTileSize) - scrollX;
    const int dy = (mY * mapTileSize) - scrollY + mapTileSize;

    MapRowVertexes *const row = new MapRowVertexes();
    mTempRows.push_back(row);
    Image *lastImage = nullptr;
    ImageVertexes *imgVert = nullptr;
    typedef std::map<int, ImageVertexes*> ImageVertexesMap;
    ImageVertexesMap imgSet;

    for (int y = startY; y < endY; y++)
    {
        const int yWidth = y * mWidth;
        const int py0 = y * mapTileSize + dy;
        TileInfo *tilePtr = &mTiles[static_cast<size_t>(startX + yWidth)];
        for (int x = startX; x < endX; x++, tilePtr++)
        {
            if (!tilePtr->isEnabled)
                continue;
            Image *const img = (*tilePtr).image;
            if (img)
            {
                const int px = x * mapTileSize + dx;
                const int py = py0 - img->mBounds.h;
                const GLuint imgGlImage = img->mGLImage;
                if (mSpecialFlag ||
                    img->mBounds.h <= mapTileSize)
                {
                    if (!lastImage ||
                        lastImage->mGLImage != imgGlImage)
                    {
                        if (img->mBounds.w > mapTileSize)
                            imgSet.clear();

                        if (imgSet.find(imgGlImage) != imgSet.end())
                        {
                            imgVert = imgSet[imgGlImage];
                        }
                        else
                        {
                            if (lastImage)
                                imgSet[lastImage->mGLImage] = imgVert;
                            imgVert = new ImageVertexes();
                            imgVert->ogl.init();
                            imgVert->image = img;
                            row->images.push_back(imgVert);
                        }
                    }
                    lastImage = img;
//                    if (imgVert->image->mGLImage != lastImage->mGLImage)
//                        logger->log("wrong image draw");
                    graphics->calcTileVertexes(imgVert, lastImage, px, py);
                }
            }
        }
    }
    FOR_EACH (MapRowImages::iterator, it, row->images)
    {
        graphics->finalize(*it);
    }
    BLOCK_END("MapLayer::updateOGL")
}

void MapLayer::drawOGL(Graphics *const graphics) const restrict2
{
    BLOCK_START("MapLayer::drawOGL")
    MapRows::const_iterator rit = mTempRows.begin();
    const MapRows::const_iterator rit_end = mTempRows.end();
//    int k = 0;
    while (rit != rit_end)
    {
        const MapRowImages *const images = &(*rit)->images;
        MapRowImages::const_iterator iit = images->begin();
        const MapRowImages::const_iterator iit_end = images->end();
        while (iit != iit_end)
        {
            graphics->drawTileVertexes(*iit);
            ++ iit;
//            k ++;
        }
        ++ rit;
    }
    BLOCK_END("MapLayer::drawOGL")
//    logger->log("draws: %d", k);
}
#endif

void MapLayer::drawFringe(Graphics *const graphics,
                          int startX,
                          int startY,
                          int endX,
                          int endY,
                          const int scrollX,
                          const int scrollY,
                          const Actors &actors) const restrict
{
    BLOCK_START("MapLayer::drawFringe")
    if (!localPlayer ||
        !mSpecialLayer ||
        !mTempLayer)
    {
        BLOCK_END("MapLayer::drawFringe")
        return;
    }

    startX -= mX;
    startY -= mY;
    endX -= mX;
    endY -= mY;

    if (startX < 0)
        startX = 0;
    if (startY < 0)
        startY = 0;
    if (endX > mWidth)
        endX = mWidth;
    if (endY > mHeight)
        endY = mHeight;

    ActorsCIter ai = actors.begin();
    const ActorsCIter ai_end = actors.end();

    const int dx = (mX * mapTileSize) - scrollX;
    const int dy = (mY * mapTileSize) - scrollY + mapTileSize;

    const int specialWidth = mSpecialLayer->mWidth;
    const int specialHeight = mSpecialLayer->mHeight;

    const bool flag = mDrawLayerFlags == MapType::SPECIAL3 ||
        mDrawLayerFlags == MapType::SPECIAL4 ||
        mDrawLayerFlags == MapType::BLACKWHITE;

    const int minEndY = std::min(specialHeight, endY);

    if (flag)
    {   // flag
        for (int y = startY; y < minEndY; y ++)
        {
            const int y32 = y * mapTileSize;
            const int y32s = (y + mActorsFix) * mapTileSize;

            BLOCK_START("MapLayer::drawFringe drawmobs")
            // If drawing the fringe layer, make sure all actors above this
            // row of tiles have been drawn
            while (ai != ai_end && (*ai)->getSortPixelY() <= y32s)
            {
                (*ai)->draw(graphics, -scrollX, -scrollY);
                ++ ai;
            }
            BLOCK_END("MapLayer::drawFringe drawmobs")

            if (y < specialHeight)
            {
                const int ptr = y * specialWidth;
                const int py1 = y32 - scrollY;
                int endX1 = endX;
                if (endX1 > specialWidth)
                    endX1 = specialWidth;
                if (endX1 < 0)
                    endX1 = 0;

                for (int x = startX; x < endX1; x++)
                {
                    const int px1 = x * mapTileSize - scrollX;

                    const MapItem *item = mSpecialLayer->mTiles[ptr + x];
                    if (item)
                    {
                        item->draw(graphics, px1, py1,
                            mapTileSize, mapTileSize);
                    }

                    item = mTempLayer->mTiles[ptr + x];
                    if (item)
                    {
                        item->draw(graphics, px1, py1,
                            mapTileSize, mapTileSize);
                    }
                }
            }
        }

        for (int y = minEndY; y < endY; y++)
        {
            const int y32s = (y + mActorsFix) * mapTileSize;

            BLOCK_START("MapLayer::drawFringe drawmobs")
            // If drawing the fringe layer, make sure all actors above this
            // row of tiles have been drawn
            while (ai != ai_end && (*ai)->getSortPixelY() <= y32s)
            {
                (*ai)->draw(graphics, -scrollX, -scrollY);
                ++ ai;
            }
            BLOCK_END("MapLayer::drawFringe drawmobs")
        }
    }
    else
    {   // !flag
        for (int y = startY; y < minEndY; y ++)
        {
            const int y32 = y * mapTileSize;
            const int y32s = (y + mActorsFix) * mapTileSize;
            const int yWidth = y * mWidth;

            BLOCK_START("MapLayer::drawFringe drawmobs")
            // If drawing the fringe layer, make sure all actors above this
            // row of tiles have been drawn
            while (ai != ai_end &&
                   (*ai)->getSortPixelY() <= y32s)
            {
                (*ai)->draw(graphics, -scrollX, -scrollY);
                ++ ai;
            }
            BLOCK_END("MapLayer::drawFringe drawmobs")

            const int py0 = y32 + dy;
            const int py1 = y32 - scrollY;

            TileInfo *tilePtr = &mTiles[static_cast<size_t>(startX + yWidth)];
            for (int x = startX; x < endX; x++, tilePtr++)
            {
                if (!tilePtr->isEnabled)
                    continue;
                const int x32 = x * mapTileSize;

                const int px1 = x32 - scrollX;
                int c = 0;
                const Image *const img = tilePtr->image;
                if (img)
                {
                    if (mSpecialFlag ||
                        img->mBounds.h <= mapTileSize)
                    {
                        const int px = x32 + dx;
                        const int py = py0 - img->mBounds.h;
                        int width = 0;
                        // here need not draw over player position
                        c = getTileDrawWidth(tilePtr, endX - x, width);

                        if (!c)
                        {
                            graphics->drawImage(img, px, py);
                        }
                        else
                        {
                            graphics->drawPattern(img, px, py,
                                width, img->mBounds.h);
                        }
                    }
                }

                if (y < specialHeight)
                {
                    int c1 = c;
                    if (c1 + x + 1 > specialWidth)
                        c1 = specialWidth - x - 1;
                    if (c1 < 0)
                        c1 = 0;

                    const int ptr = y * specialWidth + x;

                    for (int x1 = 0; x1 < c1 + 1; x1 ++)
                    {
                        const MapItem *const item1
                            = mSpecialLayer->mTiles[ptr + x1];
                        const MapItem *const item2
                            = mTempLayer->mTiles[ptr + x1];
                        if (item1 || item2)
                        {
                            const int px2 = px1 + (x1 * mapTileSize);
                            if (item1 && item1->mType != MapItemType::EMPTY)
                            {
                                item1->draw(graphics, px2, py1,
                                    mapTileSize, mapTileSize);
                            }

                            if (item2 && item2->mType != MapItemType::EMPTY)
                            {
                                item2->draw(graphics, px2, py1,
                                    mapTileSize, mapTileSize);
                            }
                        }
                    }
                }
                x += c;
                tilePtr += c;
            }
        }

        for (int y = minEndY; y < endY; y++)
        {
            const int y32 = y * mapTileSize;
            const int y32s = (y + mActorsFix) * mapTileSize;
            const int yWidth = y * mWidth;

            BLOCK_START("MapLayer::drawFringe drawmobs")
            // If drawing the fringe layer, make sure all actors above this
            // row of tiles have been drawn
            while (ai != ai_end && (*ai)->getSortPixelY() <= y32s)
            {
                (*ai)->draw(graphics, -scrollX, -scrollY);
                ++ ai;
            }
            BLOCK_END("MapLayer::drawFringe drawmobs")

            const int py0 = y32 + dy;

            TileInfo *tilePtr = &mTiles[static_cast<size_t>(startX + yWidth)];
            for (int x = startX; x < endX; x++, tilePtr++)
            {
                if (!tilePtr->isEnabled)
                    continue;
                const int x32 = x * mapTileSize;
                const Image *const img = tilePtr->image;
                if (img)
                {
                    const int px = x32 + dx;
                    const int py = py0 - img->mBounds.h;
                    if (mSpecialFlag ||
                        img->mBounds.h <= mapTileSize)
                    {
                        int width = 0;
                        // here need not draw over player position
                        const int c = getTileDrawWidth(tilePtr,
                            endX - x,
                            width);

                        if (!c)
                        {
                            graphics->drawImage(img, px, py);
                        }
                        else
                        {
                            graphics->drawPattern(img, px, py,
                                width, img->mBounds.h);
                            x += c;
                            tilePtr += c;
                        }
                    }
                }
            }
        }
    }  // !flag

    // Draw any remaining actors
    if (mDrawLayerFlags != MapType::SPECIAL3 &&
        mDrawLayerFlags != MapType::SPECIAL4)
    {
        BLOCK_START("MapLayer::drawFringe drawmobs")
        while (ai != ai_end)
        {
            (*ai)->draw(graphics, -scrollX, -scrollY);
            ++ai;
        }
        BLOCK_END("MapLayer::drawFringe drawmobs")
        if (mHighlightAttackRange)
        {
            const int px = localPlayer->getPixelX()
                - scrollX - mapTileSize / 2;
            const int py = localPlayer->getPixelY() - scrollY - mapTileSize;
            const int attackRange = localPlayer->getAttackRange()
                * mapTileSize;

            int x = px - attackRange;
            int y = py - attackRange;
            int w = 2 * attackRange + mapTileSize;
            int h = w;
            if (attackRange <= mapTileSize)
            {
                x -= mapTileSize / 2;
                y -= mapTileSize / 2;
                w += mapTileSize;
                h += mapTileSize;
            }

            if (userPalette)
            {
                graphics->setColor(userPalette->getColorWithAlpha(
                    UserColorId::ATTACK_RANGE));
                graphics->fillRectangle(Rect(x, y, w, h));
                graphics->setColor(userPalette->getColorWithAlpha(
                    UserColorId::ATTACK_RANGE_BORDER));
                graphics->drawRectangle(Rect(x, y, w, h));
            }
        }
    }
    BLOCK_END("MapLayer::drawFringe")
}

int MapLayer::getTileDrawWidth(const TileInfo *restrict tilePtr,
                               const int endX,
                               int &width)
{
    BLOCK_START("MapLayer::getTileDrawWidth")
    const Image *const img1 = tilePtr->image;
    int c = 0;
    if (!img1)
    {
        width = 0;
        BLOCK_END("MapLayer::getTileDrawWidth")
        return c;
    }
    width = img1->mBounds.w;
    for (int x = 1; x < endX; x++)
    {
        tilePtr ++;
        const Image *const img = tilePtr->image;
        if (img != img1 || !tilePtr->isEnabled)
            break;
        c ++;
        if (img)
            width += img->mBounds.w;
    }
    BLOCK_END("MapLayer::getTileDrawWidth")
    return c;
}

void MapLayer::setDrawLayerFlags(const MapTypeT &n) restrict
{
    mDrawLayerFlags = n;
    mSpecialFlag = (mDrawLayerFlags != MapType::SPECIAL &&
        mDrawLayerFlags != MapType::SPECIAL2 &&
        mDrawLayerFlags != MapType::SPECIAL4);
}

void MapLayer::updateConditionTiles(MetaTile *const metaTiles,
                                    const int width,
                                    const int height) restrict
{
    const int width1 = width < mWidth ? width : mWidth;
    const int height1 = height < mHeight ? height : mHeight;

    for (int y = mY; y < height1; y ++)
    {
        MetaTile *metaPtr = metaTiles + (y - mY) * width;
        TileInfo *tilePtr = mTiles + y * mWidth;
        for (int x = mX; x < width1; x ++, metaPtr ++, tilePtr ++)
        {
            if (metaPtr->blockmask & mTileCondition ||
                (metaPtr->blockmask == 0 &&
                mTileCondition == BlockMask::GROUND))
            {
                tilePtr->isEnabled = true;
            }
            else
            {
                tilePtr->isEnabled = false;
            }
        }
    }
}
