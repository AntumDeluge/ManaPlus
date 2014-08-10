/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#ifndef DROPSHORTCUT_H
#define DROPSHORTCUT_H

#include "shortcutbase.h"

#include "localconsts.h"

class Item;

/**
 * The class which keeps track of the item shortcuts.
 */
class DropShortcut final : public ShortcutBase
{
    public:
        /**
         * Constructor.
         */
        DropShortcut();

        A_DELETE_COPY(DropShortcut)

        /**
         * Destructor.
         */
        ~DropShortcut();

        /**
         * Drop first item.
         */
        void dropFirst() const;

        /**
         * Drop all items in cicle.
         */
        void dropItems(const int cnt = 1);

    private:
        /**
         * Drop item in cicle.
         */
        bool dropItem(const int cnt = 1);

        int mLastDropIndex;
};

extern DropShortcut *dropShortcut;

#endif  // DROPSHORTCUT_H
