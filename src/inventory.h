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

#ifndef INVENTORY_H
#define INVENTORY_H

#include "enums/inventorytype.h"

#include "enums/simpletypes/beingtypeid.h"
#include "enums/simpletypes/damaged.h"
#include "enums/simpletypes/equipm.h"
#include "enums/simpletypes/equipped.h"
#include "enums/simpletypes/favorite.h"
#include "enums/simpletypes/identified.h"
#include "enums/simpletypes/itemcolor.h"

#include "enums/being/gender.h"

#include "utils/intmap.h"

#include <list>
#include <string>

#include "localconsts.h"

class InventoryListener;
class Item;

class Inventory final
{
    public:
        A_DELETE_COPY(Inventory)

        static const int NO_SLOT_INDEX = -1; /**< Slot has no index. */

        /**
         * Constructor.
         *
         * @param size the number of items that fit in the inventory
         */
        explicit Inventory(const InventoryType::Type type,
                           const int size = -1);

        /**
         * Destructor.
         */
        ~Inventory();

        /**
         * Returns the size that this instance is configured for.
         */
        unsigned getSize() const A_WARN_UNUSED
        { return mSize; }

        /**
         * Returns the item at the specified index.
         */
        Item *getItem(const int index) const A_WARN_UNUSED;

        /**
         * Searches for the specified item by it's id.
         *
         * @param itemId The id of the item to be searched.
         * @param color The color of the item to be searched.
         * @return Item found on success, NULL on failure.
         */
        Item *findItem(const int itemId,
                       const ItemColor color) const A_WARN_UNUSED;

        /**
         * Adds a new item in a free slot.
         */
        int addItem(const int id,
                    const int type,
                    const int quantity,
                    const uint8_t refine,
                    const ItemColor color,
                    const Identified identified,
                    const Damaged damaged,
                    const Favorite favorite,
                    const Equipm equipment,
                    const Equipped equipped);

        /**
         * Sets the item at the given position.
         */
        void setItem(const int index,
                     const int id,
                     const int type,
                     const int quantity,
                     const uint8_t refine,
                     const ItemColor color,
                     const Identified identified,
                     const Damaged damaged,
                     const Favorite favorite,
                     const Equipm equipment,
                     const Equipped equipped);

        void setCards(const int index,
                      const int *const cards,
                      const int size) const;

        /**
         * Remove a item from the inventory.
         */
        void removeItem(const int id);

        /**
         * Remove the item at the specified index from the inventory.
         */
        void removeItemAt(const int index);

        /**
         * Checks if the given item is in the inventory.
         */
        bool contains(const Item *const item) const A_WARN_UNUSED;

        /**
         * Returns id of next free slot or -1 if all occupied.
         */
        int getFreeSlot() const A_WARN_UNUSED;

        /**
         * Reset all item slots.
         */
        void clear();

        /**
         * Get the number of slots filled with an item
         */
        int getNumberOfSlotsUsed() const A_WARN_UNUSED
        { return mUsed; }

        /**
         * Returns the index of the last occupied slot or 0 if none occupied.
         */
        int getLastUsedSlot() const A_WARN_UNUSED;

        void addInventoyListener(InventoryListener *const listener);

        void removeInventoyListener(InventoryListener *const listener);

        InventoryType::Type getType() const A_WARN_UNUSED
        { return mType; }

        bool isMainInventory() const A_WARN_UNUSED
        { return mType == InventoryType::INVENTORY; }

        const Item *findItemBySprite(std::string spritePath,
                                     const GenderT gender,
                                     const BeingTypeId race)
                                     const A_WARN_UNUSED;

        std::string getName() const A_WARN_UNUSED;

        void resize(const unsigned int newSize);

        int findIndexByTag(const int tag) const;

        bool addVirtualItem(const Item *const item,
                            int index);

        void virtualRemove(Item *const item,
                           const int amount);

        void virtualRestore(const Item *const item,
                            const int amount);

        void restoreVirtuals();

    protected:
        typedef std::list<InventoryListener*> InventoryListenerList;
        InventoryListenerList mInventoryListeners;

        void distributeSlotsChangedEvent();

        IntMap mVirtualRemove;
        InventoryType::Type mType;
        unsigned mSize; /**< The max number of inventory items */
        Item **mItems;  /**< The holder of items */
        int mUsed;      /**< THe number of slots in use */
};

#endif  // INVENTORY_H
