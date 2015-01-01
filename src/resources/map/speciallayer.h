/*
 *  The ManaPlus Client
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

#ifndef RESOURCES_MAP_SPECIALLAYER_H
#define RESOURCES_MAP_SPECIALLAYER_H

#include "position.h"

#include "localconsts.h"

class Graphics;
class MapItem;

class SpecialLayer final
{
    public:
        friend class Map;
        friend class MapLayer;

        SpecialLayer(const int width, const int height);

        A_DELETE_COPY(SpecialLayer)

        ~SpecialLayer();

        void draw(Graphics *const graphics, int startX, int startY,
                  int endX, int endY,
                  const int scrollX, const int scrollY) const;

        MapItem* getTile(const int x, const int y) const A_WARN_UNUSED;

        void setTile(const int x, const int y, MapItem *const item);

        void setTile(const int x, const int y, const int type);

        void addRoad(const Path &road);

        void clean() const;

    private:
        int mWidth;
        int mHeight;
        MapItem **mTiles;
};

#endif  // RESOURCES_MAP_SPECIALLAYER_H
