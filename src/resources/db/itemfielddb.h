/*
 *  The ManaPlus Client
 *  Copyright (C) 2016  The ManaPlus Developers
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

#ifndef RESOURCES_DB_ITEMFIELDDB_H
#define RESOURCES_DB_ITEMFIELDDB_H

#include "enums/simpletypes/skiperror.h"

#include <map>

#include "localconsts.h"

struct ItemFieldType;

namespace ItemFieldDb
{
    void load();

    void unload();

    void loadXmlFile(const std::string &fileName,
                     const SkipError skipError);

    typedef std::map<std::string, ItemFieldType*> FieldInfos;

    const FieldInfos &getRequiredFields();

    const FieldInfos &getAddFields();
}  // namespace ItemFieldDb

#endif  // RESOURCES_DB_ITEMFIELDDB_H
