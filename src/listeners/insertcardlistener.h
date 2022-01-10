/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2019  The ManaPlus Developers
 *  Copyright (C) 2019-2022  Andrei Karas
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

#ifndef LISTENERS_INSERTCARDLISTENER_H
#define LISTENERS_INSERTCARDLISTENER_H

#include "listeners/actionlistener.h"

#include "net/inventoryhandler.h"

#include "localconsts.h"

struct InsertCardListener final : public ActionListener
{
    InsertCardListener() :
        ActionListener(),
        cardIndex(0),
        itemIndex(0)
    {
    }

    A_DELETE_COPY(InsertCardListener)

    public:
        void action(const ActionEvent &event) override final
        {
            if (event.getId() == "yes" && (inventoryHandler != nullptr))
                inventoryHandler->insertCard(cardIndex, itemIndex);
        }

        int cardIndex;
        int itemIndex;
};

#endif  // LISTENERS_INSERTCARDLISTENER_H
