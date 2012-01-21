/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "net/tmwa/buysellhandler.h"

#include "actorspritemanager.h"
#include "configuration.h"
#include "event.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"
#include "shopitem.h"

#include "gui/buydialog.h"
#include "gui/buyselldialog.h"
#include "gui/selldialog.h"
#include "gui/shopwindow.h"

#include "gui/widgets/chattab.h"

#include "net/chathandler.h"
#include "net/messagein.h"
#include "net/net.h"

#include "net/ea/eaprotocol.h"

#include "net/tmwa/chathandler.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"

#include "debug.h"

extern Net::BuySellHandler *buySellHandler;

namespace TmwAthena
{

BuySellHandler::BuySellHandler()
{
    static const Uint16 _messages[] =
    {
        SMSG_NPC_BUY_SELL_CHOICE,
        SMSG_NPC_BUY,
        SMSG_NPC_SELL,
        SMSG_NPC_BUY_RESPONSE,
        SMSG_NPC_SELL_RESPONSE,
        0
    };
    handledMessages = _messages;
    buySellHandler = this;
    mBuyDialog = nullptr;
}

void BuySellHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_NPC_BUY_SELL_CHOICE:
            processNpcBuySellChoice(msg);
            break;

        case SMSG_NPC_BUY:
            processNpcBuy(msg);
            break;

        case SMSG_NPC_SELL:
            processNpcSell(msg, INVENTORY_OFFSET);
            break;

        case SMSG_NPC_BUY_RESPONSE:
            processNpcBuyResponse(msg);
            break;

        case SMSG_NPC_SELL_RESPONSE:
            processNpcSellResponse(msg);
            break;

        default:
            break;
    }

}

void BuySellHandler::processNpcBuy(Net::MessageIn &msg)
{
    msg.readInt16();  // length
    int sz = 11;
    if (serverVersion > 0)
        sz += 1;
    int n_items = (msg.getLength() - 4) / sz;
    mBuyDialog = new BuyDialog(mNpcId);
    mBuyDialog->setMoney(PlayerInfo::getAttribute(MONEY));

    for (int k = 0; k < n_items; k++)
    {
        int value = msg.readInt32();
        msg.readInt32();  // DCvalue
        msg.readInt8();  // type
        int itemId = msg.readInt16();
        unsigned char color = 1;
        if (serverVersion > 0)
            color = msg.readInt8();
        mBuyDialog->addItem(itemId, color, 0, value);
    }
}

void BuySellHandler::processNpcSellResponse(Net::MessageIn &msg)
{
    switch (msg.readInt8())
    {
        case 0:
            SERVER_NOTICE(_("Thanks for selling."))
            break;
        case 1:
        default:
            SERVER_NOTICE(_("Unable to sell."))
            break;
        case 2:
            SERVER_NOTICE(_("Unable to sell while trading."))
            break;
        case 3:
            SERVER_NOTICE(_("Unable to sell unsellable item."))
            break;
    }
}

} // namespace TmwAthena
