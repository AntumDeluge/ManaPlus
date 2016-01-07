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

#include "resources/inventory/inventory.h"

#include "logger.h"

#include "being/playerinfo.h"

#include "net/inventoryhandler.h"

#include "resources/iteminfo.h"

#include "resources/item/item.h"

#include "listeners/inventorylistener.h"

#include "utils/delete2.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <algorithm>

#include "debug.h"

namespace
{
    struct SlotUsed final
    {
        bool operator()(const Item *const item) const
        {
            return item && item->mId >= 0 && item->mQuantity > 0;
        }
        typedef Item *argument_type;
    };
}  // namespace

Inventory::Inventory(const InventoryTypeT type, const int size1) :
    mInventoryListeners(),
    mVirtualRemove(),
    mType(type),
    mSize(size1 == -1 ? static_cast<unsigned>(
          inventoryHandler->getSize(type))
          : static_cast<unsigned>(size1)),
    mItems(new Item*[mSize]),
    mUsed(0)
{
    std::fill_n(mItems, mSize, static_cast<Item*>(nullptr));
}

Inventory::~Inventory()
{
    for (unsigned i = 0; i < mSize; i++)
        delete mItems[i];

    delete [] mItems;
    mItems = nullptr;
}

Item *Inventory::getItem(const int index) const
{
    if (index < 0 || index >= static_cast<int>(mSize) || !mItems[index]
        || mItems[index]->mQuantity <= 0)
    {
        return nullptr;
    }

    return mItems[index];
}

Item *Inventory::findItem(const int itemId,
                          const ItemColor color) const
{
    for (unsigned i = 0; i < mSize; i++)
    {
        Item *const item = mItems[i];
        if (item && item->mId == itemId)
        {
            if (color == ItemColor_zero ||
                item->mColor == color ||
                (color == ItemColor_one &&
                item->mColor <= ItemColor_one))
            {
                return item;
            }
        }
    }

    return nullptr;
}

int Inventory::addItem(const int id,
                       const int type,
                       const int quantity,
                       const uint8_t refine,
                       const ItemColor color,
                       const Identified identified,
                       const Damaged damaged,
                       const Favorite favorite,
                       const Equipm equipment,
                       const Equipped equipped)
{
    const int slot = getFreeSlot();
    setItem(slot, id, type, quantity, refine, color,
        identified, damaged, favorite, equipment, equipped);
    return slot;
}

void Inventory::setItem(const int index,
                        const int id,
                        const int type,
                        const int quantity,
                        const uint8_t refine,
                        const ItemColor color,
                        const Identified identified,
                        const Damaged damaged,
                        const Favorite favorite,
                        const Equipm equipment,
                        const Equipped equipped)
{
    if (index < 0 || index >= static_cast<int>(mSize))
    {
        logger->log("Warning: invalid inventory index: %d", index);
        return;
    }

    Item *const item1 = mItems[index];
    if (!item1 && id > 0)
    {
        Item *const item = new Item(id, type, quantity, refine, color,
            identified, damaged, favorite, equipment, equipped);
        item->setInvIndex(index);
        mItems[index] = item;
        mUsed++;
        distributeSlotsChangedEvent();
    }
    else if (id > 0 && item1)
    {
        item1->setId(id, color);
        item1->setQuantity(quantity);
        item1->setRefine(refine);
        item1->setEquipment(equipment);
        item1->setIdentified(identified);
        item1->setDamaged(damaged);
        item1->setFavorite(favorite);
    }
    else if (item1)
    {
        removeItemAt(index);
    }
}

void Inventory::setCards(const int index,
                         const int *const cards,
                         const int size) const
{
    if (index < 0 || index >= static_cast<int>(mSize))
    {
        logger->log("Warning: invalid inventory index: %d", index);
        return;
    }

    Item *const item1 = mItems[index];
    item1->setCards(cards, size);
}

void Inventory::clear()
{
    for (unsigned i = 0; i < mSize; i++)
        removeItemAt(i);
}

void Inventory::removeItem(const int id)
{
    for (unsigned i = 0; i < mSize; i++)
    {
        const Item *const item = mItems[i];
        if (item && item->mId == id)
            removeItemAt(i);
    }
}

void Inventory::removeItemAt(const int index)
{
    if (!mItems[index])
        return;
    delete2(mItems[index]);
    mUsed--;
    if (mUsed < 0)  // Already at 0, no need to distribute event
        mUsed = 0;
    else
        distributeSlotsChangedEvent();
}

bool Inventory::contains(const Item *const item) const
{
    if (!item)
        return false;

    const int id = item->mId;
    for (unsigned i = 0; i < mSize; i++)
    {
        const Item *const item1 = mItems[i];
        if (item1 && item1->mId == id)
            return true;
    }

    return false;
}

int Inventory::getFreeSlot() const
{
    Item **const i = std::find_if(mItems, mItems + mSize,
        std::not1(SlotUsed()));
    return (i == mItems + mSize) ? -1
        : static_cast<int>(i - mItems);
}

int Inventory::getLastUsedSlot() const
{
    for (int i = mSize - 1; i >= 0; i--)
    {
        if (SlotUsed()(mItems[i]))
            return i;
    }

    return -1;
}

void Inventory::addInventoyListener(InventoryListener* const listener)
{
    mInventoryListeners.push_back(listener);
}

void Inventory::removeInventoyListener(InventoryListener* const listener)
{
    mInventoryListeners.remove(listener);
}

void Inventory::distributeSlotsChangedEvent()
{
    FOR_EACH (InventoryListenerList::const_iterator, i, mInventoryListeners)
        (*i)->slotsChanged(this);
}

const Item *Inventory::findItemBySprite(std::string spritePath,
                                        const GenderT gender,
                                        const BeingTypeId race) const
{
    spritePath = removeSpriteIndex(spritePath);

    const std::string spritePathShort = extractNameFromSprite(spritePath);
    int partialIndex = -1;

    for (unsigned i = 0; i < mSize; i++)
    {
        const Item *const item = mItems[i];
        if (item)
        {
            std::string path = item->getInfo().getSprite(gender, race);
            if (!path.empty())
            {
                path = removeSpriteIndex(path);
                if (spritePath == path)
                    return item;

                path = extractNameFromSprite(path);
                if (spritePathShort == path)
                    partialIndex = i;
            }
        }
    }
    if (partialIndex != -1)
        return mItems[partialIndex];

    return nullptr;
}

std::string Inventory::getName() const
{
    switch (mType)
    {
        case InventoryType::Inventory:
#ifdef EATHENA_SUPPORT
        case InventoryType::Vending:
#endif
        case InventoryType::TypeEnd:
        default:
        {
            // TRANSLATORS: inventory type name
            return N_("Inventory");
        }
        case InventoryType::Storage:
        {
            // TRANSLATORS: inventory type name
            return N_("Storage");
        }
        case InventoryType::Npc:
        {
            // TRANSLATORS: inventory type name
            return N_("Npc");
        }
#ifdef EATHENA_SUPPORT
        case InventoryType::Cart:
        {
            // TRANSLATORS: inventory type name
            return N_("Cart");
        }
        case InventoryType::Mail:
        {
            // TRANSLATORS: inventory type name
            return N_("Mail");
        }
        case InventoryType::Craft:
        {
            // TRANSLATORS: inventory type name
            return N_("Craft");
        }
#endif
        case InventoryType::Trade:
        {
            // TRANSLATORS: inventory type name
            return N_("Trade");
        }
    }
}

void Inventory::resize(const unsigned int newSize)
{
    clear();
    if (mSize == newSize)
        return;

    for (unsigned i = 0; i < mSize; i++)
        delete mItems[i];
    delete [] mItems;

    mSize = newSize;
    mItems = new Item*[static_cast<size_t>(mSize)];
    std::fill_n(mItems, mSize, static_cast<Item*>(nullptr));
}

int Inventory::findIndexByTag(const int tag) const
{
    for (unsigned i = 0; i < mSize; i++)
    {
        const Item *const item = mItems[i];
        if (item && item->mTag == tag)
            return i;
    }

    return -1;
}

bool Inventory::addVirtualItem(const Item *const item,
                               int index)
{
    if (item && !PlayerInfo::isItemProtected(item->getId()))
    {
        if (index >= 0 && index < static_cast<int>(mSize))
        {
            if (mItems[index] != nullptr)
                return false;
            setItem(index,
                item->getId(),
                item->getType(),
                1,
                1,
                item->getColor(),
                item->getIdentified(),
                item->getDamaged(),
                item->getFavorite(),
                Equipm_false,
                Equipped_false);
        }
        else
        {
            index = addItem(item->getId(),
                item->getType(),
                1,
                1,
                item->getColor(),
                item->getIdentified(),
                item->getDamaged(),
                item->getFavorite(),
                Equipm_false,
                Equipped_false);
        }
        if (index == -1)
            return false;

        Item *const item2 = getItem(index);
        if (item2)
            item2->setTag(item->getInvIndex());
        return true;
    }
    return false;
}

void Inventory::virtualRemove(Item *const item,
                              const int amount)
{
    if (!item || item->mQuantity < amount)
        return;

    const int index = item->getInvIndex();
    const IntMapCIter it = mVirtualRemove.find(index);
    if (it == mVirtualRemove.end())
        mVirtualRemove[index] = amount;
    else
        mVirtualRemove[index] += amount;
    item->mQuantity -= amount;
}

void Inventory::restoreVirtuals()
{
    const int sz = static_cast<int>(mSize);

    logger->log("Inventory::restoreVirtuals 1");
    FOR_EACH (IntMapCIter, it, mVirtualRemove)
    {
        logger->log("Inventory::restoreVirtuals 2");
        const int index = (*it).first;
        if (index < 0 || index >= sz)
            continue;
        Item *const item = mItems[index];
        if (!item)
            continue;
        item->mQuantity += (*it).second;
    }
    mVirtualRemove.clear();
}

void Inventory::virtualRestore(const Item *const item,
                               const int amount)
{
    const int index = item->getTag();
    const IntMapCIter it = mVirtualRemove.find(index);
    if (it != mVirtualRemove.end())
    {
        mVirtualRemove[index] -= amount;
        if (mVirtualRemove[index] < 0)
            mVirtualRemove.erase(index);
        if (index < 0 || index >= static_cast<int>(mSize) || !mItems[index])
            return;
        mItems[index]->mQuantity += amount;
    }
}
