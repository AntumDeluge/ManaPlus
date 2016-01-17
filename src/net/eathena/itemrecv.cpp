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

#include "net/eathena/itemrecv.h"

#include "actormanager.h"
#include "itemcolormanager.h"
#include "logger.h"

#include "const/resources/item/cards.h"

#include "net/messagein.h"

#include "debug.h"

namespace EAthena
{

void ItemRecv::processItemDropped(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("id");
    const int itemId = msg.readInt16("item id");
    const int itemType = msg.readInt16("type");
    const Identified identified = fromInt(
        msg.readUInt8("identify"), Identified);
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    const int subX = static_cast<int>(msg.readInt8("subx"));
    const int subY = static_cast<int>(msg.readInt8("suby"));
    const int amount = msg.readInt16("count");

    if (actorManager)
    {
        actorManager->createItem(id,
            itemId,
            x, y,
            itemType,
            amount,
            0,
            ItemColor_one,
            identified,
            Damaged_false,
            subX, subY,
            nullptr);
    }
}

void ItemRecv::processItemDropped2(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("id");
    const int itemId = msg.readInt16("item id");
    const int itemType = msg.readUInt8("type");
    const Identified identified = fromInt(
        msg.readUInt8("identify"), Identified);
    const Damaged damaged = fromBool(msg.readUInt8("attribute"), Damaged);
    const uint8_t refine = msg.readUInt8("refine");
    int cards[maxCards];
    for (int f = 0; f < maxCards; f++)
        cards[f] = msg.readInt16("card");
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    const int amount = msg.readInt16("amount");
    const int subX = static_cast<int>(msg.readInt8("subx"));
    const int subY = static_cast<int>(msg.readInt8("suby"));

    if (actorManager)
    {
        actorManager->createItem(id,
            itemId,
            x, y,
            itemType,
            amount,
            refine,
            ItemColorManager::getColorFromCards(&cards[0]),
            identified,
            damaged,
            subX, subY,
            &cards[0]);
    }
}

void ItemRecv::processItemMvpDropped(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("len");
    msg.readUInt8("type");
    msg.readInt16("item id");
    msg.readUInt8("len");
    msg.readString(24, "name");
    msg.readUInt8("monster name len");
    msg.readString(24, "monster name");
}

void ItemRecv::processItemVisible(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("item object id");
    const int itemId = msg.readInt16("item id");
    const Identified identified = fromInt(
        msg.readUInt8("identify"), Identified);
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    const int amount = msg.readInt16("amount");
    const int subX = static_cast<int>(msg.readInt8("sub x"));
    const int subY = static_cast<int>(msg.readInt8("sub y"));

    if (actorManager)
    {
        actorManager->createItem(id,
            itemId,
            x, y,
            0,
            amount,
            0,
            ItemColor_one,
            identified,
            Damaged_false,
            subX, subY,
            nullptr);
    }
}

void ItemRecv::processItemVisible2(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("item object id");
    const int itemId = msg.readInt16("item id");
    const int itemType = msg.readUInt8("type");
    const Identified identified = fromInt(
        msg.readUInt8("identify"), Identified);
    const Damaged damaged = fromBool(msg.readUInt8("attribute"), Damaged);
    const uint8_t refine = msg.readUInt8("refine");
    int cards[maxCards];
    for (int f = 0; f < maxCards; f++)
        cards[f] = msg.readInt16("card");
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    const int amount = msg.readInt16("amount");
    const int subX = static_cast<int>(msg.readInt8("sub x"));
    const int subY = static_cast<int>(msg.readInt8("sub y"));

    if (actorManager)
    {
        actorManager->createItem(id,
            itemId,
            x, y,
            itemType,
            amount,
            refine,
            ItemColorManager::getColorFromCards(&cards[0]),
            identified,
            damaged,
            subX, subY,
            &cards[0]);
    }
}

}  // namespace EAthena
