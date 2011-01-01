/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DROPSHORTCUT_H
#define DROPSHORTCUT_H

#define DROP_SHORTCUT_ITEMS 16

class Item;

/**
 * The class which keeps track of the item shortcuts.
 */
class DropShortcut
{
    public:
        /**
         * Constructor.
         */
        DropShortcut();

        /**
         * Destructor.
         */
        ~DropShortcut();

        /**
         * Load the configuration information.
         */
        void load(bool oldConfig = false);

        /**
         * Save the configuration information.
         */
        void save();

        /**
         * Returns the shortcut item ID specified by the index.
         *
         * @param index Index of the shortcut item.
         */
        int getItem(int index) const
        { return mItems[index]; }

        /**
         * Returns the amount of shortcut items.
         */
        int getItemCount() const
        { return DROP_SHORTCUT_ITEMS; }

        /**
         * Returns the item ID that is currently selected.
         */
        int getItemSelected() const
        { return mItemSelected; }

        /**
         * Adds the selected item ID to the items specified by the index.
         *
         * @param index Index of the items.
         */
        void setItem(int index)
        { mItems[index] = mItemSelected; save(); }

        /**
         * Adds an item to the items store specified by the index.
         *
         * @param index Index of the item.
         * @param itemId ID of the item.
         */
        void setItems(int index, int itemId)
        { mItems[index] = itemId; save(); }

        /**
         * Set the item that is selected.
         *
         * @param itemId The ID of the item that is to be assigned.
         */
        void setItemSelected(int itemId)
        { mItemSelected = itemId; }

        /**
         * A flag to check if the item is selected.
         */
        bool isItemSelected() const
        { return mItemSelected > -1; }

        /**
         * Remove a item from the shortcut.
         */
        void removeItem(int index)
        { mItems[index] = -1; save(); }

        /**
         * Try to use the item specified by the index.
         *
         * @param index Index of the item shortcut.
         */
        void useItem(int index);

        /**
         * Equip a item from the shortcut.
         */
        void equipItem(int index);

        /**
         * UnEquip a item from the shortcut.
         */
        void unequipItem(int index);

        /**
         * Drop first item.
         */
        void dropFirst();

        /**
         * Drop all items in cicle.
         */
        void dropItems(int cnt = 1);

    private:

        /**
         * Drop item in cicle.
         */
        bool dropItem(int cnt = 1);

        int mItems[DROP_SHORTCUT_ITEMS];     /**< The items stored. */
        int mItemSelected;              /**< The item held by cursor. */

        int mLastDropIndex;
};

extern DropShortcut *dropShortcut;

#endif
