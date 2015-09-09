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

#include "net/tmwa/inventoryrecv.h"

#include "notifymanager.h"

#include "being/localplayer.h"

#include "enums/resources/notifytypes.h"

#include "listeners/arrowslistener.h"

#include "net/messagein.h"

#include "net/ea/eaprotocol.h"
#include "net/ea/equipbackend.h"
#include "net/ea/inventoryrecv.h"

#include "debug.h"

namespace TmwAthena
{

void InventoryRecv::processPlayerEquipment(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerEquipment")
    Inventory *const inventory = localPlayer
        ? PlayerInfo::getInventory() : nullptr;

    msg.readInt16("len");
    Equipment *const equipment = PlayerInfo::getEquipment();
    if (equipment && !equipment->getBackend())
    {   // look like SMSG_PLAYER_INVENTORY was not received
        Ea::InventoryRecv::mEquips.clear();
        equipment->setBackend(&Ea::InventoryRecv::mEquips);
    }
    const int number = (msg.getLength() - 4) / 20;

    for (int loop = 0; loop < number; loop++)
    {
        const int index = msg.readInt16("index") - INVENTORY_OFFSET;
        const int itemId = msg.readInt16("item id");
        const uint8_t itemType = msg.readUInt8("item type");
        const uint8_t identified = msg.readUInt8("identify");

        msg.readInt16("equip type?");
        const int equipType = msg.readInt16("equip type");
        msg.readUInt8("attribute");
        const uint8_t refine = msg.readUInt8("refine");
        int cards[4];
        for (int f = 0; f < 4; f++)
            cards[f] = msg.readInt16("card");

        if (Ea::InventoryRecv::mDebugInventory)
        {
            logger->log("Index: %d, ID: %d, Type: %d, Identified: %d",
                        index, itemId, itemType, identified);
        }

        if (inventory)
        {
            inventory->setItem(index,
                itemId,
                itemType,
                1,
                refine,
                ItemColor_one,
                fromBool(identified, Identified),
                Damaged_false,
                Favorite_false,
                Equipm_true,
                Equipped_false);
            inventory->setCards(index, cards, 4);
        }

        if (equipType)
        {
            Ea::InventoryRecv::mEquips.setEquipment(
                Ea::InventoryRecv::getSlot(equipType), index);
        }
    }
    BLOCK_END("InventoryRecv::processPlayerEquipment")
}

void InventoryRecv::processPlayerInventoryAdd(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerInventoryAdd")
    Inventory *const inventory = localPlayer
        ? PlayerInfo::getInventory() : nullptr;

    if (PlayerInfo::getEquipment()
        && !PlayerInfo::getEquipment()->getBackend())
    {   // look like SMSG_PLAYER_INVENTORY was not received
        Ea::InventoryRecv::mEquips.clear();
        PlayerInfo::getEquipment()->setBackend(&Ea::InventoryRecv::mEquips);
    }
    const int index = msg.readInt16("index") - INVENTORY_OFFSET;
    int amount = msg.readInt16("amount");
    const int itemId = msg.readInt16("item id");
    const uint8_t identified = msg.readUInt8("identified");
    msg.readUInt8("attribute");
    const uint8_t refine = msg.readUInt8("refine");
    int cards[4];
    for (int f = 0; f < 4; f++)
        cards[f] = msg.readInt16("card");
    const int equipType = msg.readInt16("equip type");
    const int type = msg.readUInt8("item type");

    const ItemInfo &itemInfo = ItemDB::get(itemId);
    const unsigned char err = msg.readUInt8("status");
    BeingId floorId;
    if (Ea::InventoryRecv::mSentPickups.empty())
    {
        floorId = BeingId_zero;
    }
    else
    {
        floorId = Ea::InventoryRecv::mSentPickups.front();
        Ea::InventoryRecv::mSentPickups.pop();
    }

    if (err)
    {
        PickupT pickup;
        switch (err)
        {
            case 1:
                pickup = Pickup::BAD_ITEM;
                break;
            case 2:
                pickup = Pickup::TOO_HEAVY;
                break;
            case 3:
                pickup = Pickup::TOO_FAR;
                break;
            case 4:
                pickup = Pickup::INV_FULL;
                break;
            case 5:
                pickup = Pickup::STACK_FULL;
                break;
            case 6:
                pickup = Pickup::DROP_STEAL;
                break;
            default:
                pickup = Pickup::UNKNOWN;
                UNIMPLIMENTEDPACKET;
                break;
        }
        if (localPlayer)
        {
            localPlayer->pickedUp(itemInfo,
                0,
                ItemColor_one,
                floorId,
                pickup);
        }
    }
    else
    {
        if (localPlayer)
        {
            localPlayer->pickedUp(itemInfo,
                amount,
                ItemColor_one,
                floorId,
                Pickup::OKAY);
        }

        if (inventory)
        {
            const Item *const item = inventory->getItem(index);

            if (item && item->getId() == itemId)
                amount += item->getQuantity();

            inventory->setItem(index,
                itemId,
                type,
                amount,
                refine,
                ItemColor_one,
                fromBool(identified, Identified),
                Damaged_false,
                Favorite_false,
                fromBool(equipType, Equipm),
                Equipped_false);
            inventory->setCards(index, cards, 4);
        }
        ArrowsListener::distributeEvent();
    }
    BLOCK_END("InventoryRecv::processPlayerInventoryAdd")
}

void InventoryRecv::processPlayerInventory(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerInventory")
    Inventory *const inventory = localPlayer
        ? PlayerInfo::getInventory() : nullptr;

    if (PlayerInfo::getEquipment())
    {
        // Clear inventory - this will be a complete refresh
        Ea::InventoryRecv::mEquips.clear();
        PlayerInfo::getEquipment()->setBackend(&Ea::InventoryRecv::mEquips);
    }

    if (inventory)
        inventory->clear();

    msg.readInt16("len");
    const int number = (msg.getLength() - 4) / 18;

    for (int loop = 0; loop < number; loop++)
    {
        int cards[4];
        const int index = msg.readInt16("index") - INVENTORY_OFFSET;
        const int itemId = msg.readInt16("item id");
        const uint8_t itemType = msg.readUInt8("item type");
        const uint8_t identified = msg.readUInt8("identified");
        const int amount = msg.readInt16("amount");
        const int arrow = msg.readInt16("arrow");
        for (int i = 0; i < 4; i++)
            cards[i] = msg.readInt16("card");

        if (Ea::InventoryRecv::mDebugInventory)
        {
            logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                        "Qty: %d, Cards: %d, %d, %d, %d",
                        index, itemId, itemType, identified, amount,
                        cards[0], cards[1], cards[2], cards[3]);
        }

        // Trick because arrows are not considered equipment
        const bool isEquipment = arrow & 0x8000;

        if (inventory)
        {
            inventory->setItem(index,
                itemId,
                itemType,
                amount,
                0,
                ItemColor_one,
                fromBool(identified, Identified),
                Damaged_false,
                Favorite_false,
                fromBool(isEquipment, Equipm),
                Equipped_false);
            inventory->setCards(index, cards, 4);
        }
    }
    BLOCK_END("InventoryRecv::processPlayerInventory")
}

void InventoryRecv::processPlayerStorage(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerInventory")
    Ea::InventoryRecv::mInventoryItems.clear();

    msg.readInt16("len");
    const int number = (msg.getLength() - 4) / 18;

    for (int loop = 0; loop < number; loop++)
    {
        int cards[4];
        const int index = msg.readInt16("index") - STORAGE_OFFSET;
        const int itemId = msg.readInt16("item id");
        const uint8_t itemType = msg.readUInt8("item type");
        const uint8_t identified = msg.readUInt8("identified");
        const int amount = msg.readInt16("amount");
        msg.readInt16("arrow");
        for (int i = 0; i < 4; i++)
            cards[i] = msg.readInt16("card");

        if (Ea::InventoryRecv::mDebugInventory)
        {
            logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                        "Qty: %d, Cards: %d, %d, %d, %d",
                        index, itemId, itemType, identified, amount,
                        cards[0], cards[1], cards[2], cards[3]);
        }

        Ea::InventoryRecv::mInventoryItems.push_back(Ea::InventoryItem(
            index,
            itemId,
            itemType,
            cards,
            amount,
            0,
            ItemColor_one,
            fromBool(identified, Identified),
            Damaged_false,
            Favorite_false,
            Equipm_false));
    }
    BLOCK_END("InventoryRecv::processPlayerInventory")
}

void InventoryRecv::processPlayerEquip(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerEquip")
    const int index = msg.readInt16("index") - INVENTORY_OFFSET;
    const int equipType = msg.readInt16("equip type");
    const uint8_t flag = msg.readUInt8("flag");

    if (!flag)
    {
        NotifyManager::notify(NotifyTypes::EQUIP_FAILED);
    }
    else
    {
        Ea::InventoryRecv::mEquips.setEquipment(
            Ea::InventoryRecv::getSlot(equipType), index);
    }
    BLOCK_END("InventoryRecv::processPlayerEquip")
}

void InventoryRecv::processPlayerUnEquip(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerUnEquip")
    msg.readInt16("index");
    const int equipType = msg.readInt16("equip type");
    const uint8_t flag = msg.readUInt8("flag");

    if (flag)
    {
        Ea::InventoryRecv::mEquips.setEquipment(
            Ea::InventoryRecv::getSlot(equipType), -1);
    }
    if (equipType & 0x8000)
        ArrowsListener::distributeEvent();
    BLOCK_END("InventoryRecv::processPlayerUnEquip")
}

void InventoryRecv::processPlayerStorageEquip(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerStorageEquip")
    msg.readInt16("len");
    const int number = (msg.getLength() - 4) / 20;

    for (int loop = 0; loop < number; loop++)
    {
        int cards[4];
        const int index = msg.readInt16("index") - STORAGE_OFFSET;
        const int itemId = msg.readInt16("item id");
        const uint8_t itemType = msg.readUInt8("item type");
        const uint8_t identified = msg.readUInt8("identified");
        const int amount = 1;
        msg.readInt16("equip point?");
        msg.readInt16("another equip point?");
        msg.readUInt8("attribute (broken)");
        const uint8_t refine = msg.readUInt8("refine");
        for (int i = 0; i < 4; i++)
            cards[i] = msg.readInt16("card");

        if (Ea::InventoryRecv::mDebugInventory)
        {
            logger->log("Index: %d, ID: %d, Type: %d, Identified: %u, "
                "Qty: %d, Cards: %d, %d, %d, %d, Refine: %u",
                index, itemId, itemType,
                static_cast<unsigned int>(identified), amount,
                cards[0], cards[1], cards[2], cards[3],
                static_cast<unsigned int>(refine));
        }

        Ea::InventoryRecv::mInventoryItems.push_back(Ea::InventoryItem(
            index,
            itemId,
            itemType,
            cards,
            amount,
            refine,
            ItemColor_one,
            fromBool(identified, Identified),
            Damaged_false,
            Favorite_false,
            Equipm_false));
    }
    BLOCK_END("InventoryRecv::processPlayerStorageEquip")
}

void InventoryRecv::processPlayerStorageAdd(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerStorageAdd")
    // Move an item into storage
    const int index = msg.readInt16("index") - STORAGE_OFFSET;
    const int amount = msg.readInt32("amount");
    const int itemId = msg.readInt16("item id");
    const unsigned char identified = msg.readUInt8("identified");
    msg.readUInt8("attribute");
    const uint8_t refine = msg.readUInt8("refine");
    int cards[4];
    for (int f = 0; f < 4; f++)
        cards[f] = msg.readInt16("card");

    if (Item *const item = Ea::InventoryRecv::mStorage->getItem(index))
    {
        item->setId(itemId, ItemColor_one);
        item->increaseQuantity(amount);
    }
    else
    {
        if (Ea::InventoryRecv::mStorage)
        {
            Ea::InventoryRecv::mStorage->setItem(index,
                itemId,
                0,
                amount,
                refine,
                ItemColor_one,
                fromBool(identified, Identified),
                Damaged_false,
                Favorite_false,
                Equipm_false,
                Equipped_false);
            Ea::InventoryRecv::mStorage->setCards(index, cards, 4);
        }
    }
    BLOCK_END("InventoryRecv::processPlayerStorageAdd")
}

void InventoryRecv::processPlayerStorageRemove(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerStorageRemove")
    // Move an item out of storage
    const int index = msg.readInt16("index") - STORAGE_OFFSET;
    const int amount = msg.readInt16("amount");
    if (Ea::InventoryRecv::mStorage)
    {
        if (Item *const item = Ea::InventoryRecv::mStorage->getItem(index))
        {
            item->increaseQuantity(-amount);
            if (item->getQuantity() == 0)
                Ea::InventoryRecv::mStorage->removeItemAt(index);
        }
    }
    BLOCK_END("InventoryRecv::processPlayerStorageRemove")
}

void InventoryRecv::processPlayerInventoryRemove(Net::MessageIn &msg)
{
    BLOCK_START("InventoryRecv::processPlayerInventoryRemove")
    Inventory *const inventory = localPlayer
        ? PlayerInfo::getInventory() : nullptr;

    const int index = msg.readInt16("index") - INVENTORY_OFFSET;
    const int amount = msg.readInt16("amount");
    if (inventory)
    {
        if (Item *const item = inventory->getItem(index))
        {
            item->increaseQuantity(-amount);
            if (item->getQuantity() == 0)
                inventory->removeItemAt(index);
            ArrowsListener::distributeEvent();
        }
    }
    BLOCK_END("InventoryRecv::processPlayerInventoryRemove")
}

}  // namespace TmwAthena
