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

#include "net/eathena/buysellrecv.h"

#include "notifymanager.h"

#include "being/playerinfo.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/buydialog.h"

#include "gui/widgets/createwidget.h"

#include "net/ea/buysellrecv.h"

#include "net/messagein.h"

#include "debug.h"

namespace EAthena
{

void BuySellRecv::processNpcBuy(Net::MessageIn &msg)
{
    msg.readInt16("len");
    const int sz = 11;
    const int n_items = (msg.getLength() - 4) / sz;
    CREATEWIDGETV(Ea::BuySellRecv::mBuyDialog, BuyDialog,
        Ea::BuySellRecv::mNpcId);
    Ea::BuySellRecv::mBuyDialog->setMoney(
        PlayerInfo::getAttribute(Attributes::MONEY));

    for (int k = 0; k < n_items; k++)
    {
        const int value = msg.readInt32("price");
        msg.readInt32("dc value?");
        const int type = msg.readUInt8("type");
        const int itemId = msg.readInt16("item id");
        const ItemColor color = ItemColor_one;
        Ea::BuySellRecv::mBuyDialog->addItem(itemId, type, color, 0, value);
    }
    Ea::BuySellRecv::mBuyDialog->sort();
}

void BuySellRecv::processNpcSellResponse(Net::MessageIn &msg)
{
    switch (msg.readUInt8("result"))
    {
        case 0:
            NotifyManager::notify(NotifyTypes::SOLD);
            break;
        case 1:
        default:
            NotifyManager::notify(NotifyTypes::SELL_FAILED);
            break;
        case 2:
            NotifyManager::notify(NotifyTypes::SELL_TRADE_FAILED);
            break;
        case 3:
            NotifyManager::notify(NotifyTypes::SELL_UNSELLABLE_FAILED);
            break;
    }
}

}  // namespace EAthena
