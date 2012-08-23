/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "net/ea/itemhandler.h"

#include "actorspritemanager.h"

#include "debug.h"

namespace Ea
{

ItemHandler::ItemHandler()
{
}

void ItemHandler::processItemVisible(Net::MessageIn &msg)
{
    int id = msg.readInt32();
    int itemId = msg.readInt16();
    unsigned char identify = msg.readInt8();  // identify flag
    int x = msg.readInt16();
    int y = msg.readInt16();
    int amount = msg.readInt16();
    int subX = msg.readInt8();
    int subY = msg.readInt8();

    if (actorSpriteManager)
    {
        actorSpriteManager->createItem(id, itemId,
            x, y, amount, identify, subX, subY);
    }
}

void ItemHandler::processItemDropped(Net::MessageIn &msg)
{
    int id = msg.readInt32();
    int itemId = msg.readInt16();
    unsigned char identify = msg.readInt8();  // identify flag
    int x = msg.readInt16();
    int y = msg.readInt16();
    int subX = msg.readInt8();
    int subY = msg.readInt8();
    int amount = msg.readInt16();

    if (actorSpriteManager)
    {
        actorSpriteManager->createItem(id, itemId,
            x, y, amount, identify, subX, subY);
    }
}

void ItemHandler::processItemRemove(Net::MessageIn &msg)
{
    if (actorSpriteManager)
    {
        if (FloorItem *item = actorSpriteManager->findItem(msg.readInt32()))
            actorSpriteManager->destroy(item);
    }
}

} // namespace Ea
