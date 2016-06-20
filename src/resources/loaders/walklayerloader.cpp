/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2016  The ManaPlus Developers
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

#include "resources/resourcemanager/resourcemanager.h"

#include "navigationmanager.h"

#include "resources/map/walklayer.h"

#include "resources/loaders/walklayerloader.h"

#include "utils/checkutils.h"

#include "debug.h"

#ifndef DYECMD
struct WalkLayerLoader final
{
    const std::string name;
    Map *map;

    static Resource *load(const void *const v)
    {
        if (!v)
            return nullptr;

        const WalkLayerLoader *const rl = static_cast<const
            WalkLayerLoader *const>(v);
        Resource *const resource = NavigationManager::loadWalkLayer(rl->map);
        if (!resource)
            reportAlways("WalkLayer creation error");
        return resource;
    }
};

WalkLayer *Loader::getWalkLayer(const std::string &name,
                                Map *const map)
{
    WalkLayerLoader rl = {name, map};
    return static_cast<WalkLayer*>(resourceManager->get("map_" + name,
        WalkLayerLoader::load, &rl));
}
#else
WalkLayer *Loader::getWalkLayer(const std::string &name A_UNUSED,
                                Map *const map A_UNUSED)
{
    return nullptr;
}
#endif
