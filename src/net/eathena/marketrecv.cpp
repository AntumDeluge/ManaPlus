/*
 *  The ManaPlus Client
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

#include "net/eathena/marketrecv.h"

#include "notifymanager.h"

#include "being/playerinfo.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/buydialog.h"

#include "gui/widgets/createwidget.h"

#include "net/messagein.h"

#include "debug.h"

namespace EAthena
{

namespace MarketRecv
{
    BuyDialog *mBuyDialog = nullptr;
}  // namespace MarketRecv


void MarketRecv::processMarketOpen(Net::MessageIn &msg)
{
    const int len = (msg.readInt16("len") - 4) / 13;

    CREATEWIDGETV(mBuyDialog, BuyDialog, fromInt(BuyDialog::Market, BeingId));
    mBuyDialog->setMoney(PlayerInfo::getAttribute(Attributes::MONEY));

    for (int f = 0; f < len; f ++)
    {
        const int itemId = msg.readInt16("item id");
        const ItemTypeT type = fromInt(msg.readUInt8("type"), ItemTypeT);
        const int value = msg.readInt32("price");
        const int amount = msg.readInt32("amount");
        msg.readInt16("view");
        const ItemColor color = ItemColor_one;
        mBuyDialog->addItem(itemId, type, color, amount, value);
    }
    mBuyDialog->sort();
}

void MarketRecv::processMarketBuyAck(Net::MessageIn &msg)
{
    const int len = (msg.readInt16("len") - 5) / 8;
    const int res = msg.readUInt8("result");
    for (int f = 0; f < len; f ++)
    {
        msg.readInt16("item id");
        msg.readInt16("amount");
        msg.readInt32("price");
    }
    if (res)
        NotifyManager::notify(NotifyTypes::BUY_DONE);
    else
        NotifyManager::notify(NotifyTypes::BUY_FAILED);
}

}  // namespace EAthena
