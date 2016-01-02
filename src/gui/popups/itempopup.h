/*
 *  The ManaPlus Client
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#ifndef GUI_POPUPS_ITEMPOPUP_H
#define GUI_POPUPS_ITEMPOPUP_H

#include "enums/simpletypes/itemcolor.h"

#include "gui/widgets/popup.h"

#include "resources/itemtype.h"

class Icon;
class Item;
class ItemInfo;
class Label;
class TextBox;

/**
 * A popup that displays information about an item.
 */
class ItemPopup final : public Popup
{
    public:
        /**
         * Constructor. Initializes the item popup.
         */
        ItemPopup();

        A_DELETE_COPY(ItemPopup)

        /**
         * Destructor. Cleans up the item popup on deletion.
         */
        ~ItemPopup();

        void postInit() override final;

        /**
         * Sets the info to be displayed given a particular item.
         */
        void setItem(const ItemInfo &item,
                     const ItemColor color,
                     const bool showImage,
                     int id,
                     const int *const cards);

        void setItem(const Item *const item,
                     const bool showImage);

        void mouseMoved(MouseEvent &event) override final;

        void resetPopup();

    private:
#ifdef EATHENA_SUPPORT
        std::string getCardsString(const int *const cards);
#endif

        Label *mItemName A_NONNULLPOINTER;
        TextBox *mItemDesc A_NONNULLPOINTER;
        TextBox *mItemEffect A_NONNULLPOINTER;
        TextBox *mItemWeight A_NONNULLPOINTER;
        TextBox *mItemCards A_NONNULLPOINTER;
        ItemType::Type mItemType;
        Icon *mIcon A_NONNULLPOINTER;
        std::string mLastName;
        int mLastId;
        ItemColor mLastColor;

        void setLabelColor(Label *label,
                           const ItemType::Type type) const A_NONNULL(2);
};

extern ItemPopup *itemPopup;

#endif  // GUI_POPUPS_ITEMPOPUP_H
