/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef RESOURCES_DB_HORSEDB_H
#define RESOURCES_DB_HORSEDB_H

#include <map>
#include <string>

#include "localconsts.h"

class AnimatedSprite;

struct HorseInfo;

typedef std::map<int, HorseInfo*> HorseInfos;

/**
 * Horse information database.
 */
namespace HorseDB
{
    void load();

    void loadXmlFile(const std::string &fileName);

    void loadSpecialXmlFile(const std::string &fileName);

    void unload();

    HorseInfo *get(const int id,
                   const bool allowNull = false) A_WARN_UNUSED;

    int size();

    typedef HorseInfos::iterator HorseInfosIterator;
}  // namespace HorseDB

#endif  // RESOURCES_DB_HORSEDB_H
