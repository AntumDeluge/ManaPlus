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

#include "net/ea/tradehandler.h"

#include "notifymanager.h"

#include "being/playerinfo.h"
#include "being/playerrelations.h"

#include "gui/windows/confirmdialog.h"
#include "gui/windows/tradewindow.h"

#include "net/messagein.h"

#include "utils/gettext.h"

#include "resources/notifytypes.h"

#include "listeners/requesttradelistener.h"

#include "debug.h"

extern std::string tradePartnerName;
ConfirmDialog *confirmDlg = nullptr;

/**
 * Listener for request trade dialogs
 */
namespace
{
    RequestTradeListener listener;
}  // namespace

namespace Ea
{

TradeHandler::TradeHandler()
{
    confirmDlg = nullptr;
}

void TradeHandler::removeItem(const int slotNum A_UNUSED,
                              const int amount A_UNUSED) const
{
}

void TradeHandler::processTradeResponseContinue(const uint8_t type)
{
    switch (type)
    {
        case 0:  // Too far away
            NotifyManager::notify(NotifyTypes::TRADE_FAIL_FAR_AWAY,
                tradePartnerName);
            break;
        case 1:  // Character doesn't exist
            NotifyManager::notify(NotifyTypes::TRADE_FAIL_CHAR_NOT_EXISTS,
                tradePartnerName);
            break;
        case 2:  // Invite request check failed...
            NotifyManager::notify(NotifyTypes::TRADE_CANCELLED_ERROR);
            break;
        case 3:  // Trade accepted
            if (tradeWindow)
            {
                tradeWindow->reset();
                // TRANSLATORS: trade header
                tradeWindow->setCaption(strprintf(_("Trade: You and %s"),
                    tradePartnerName.c_str()));
                tradeWindow->initTrade(tradePartnerName);
                tradeWindow->setVisible(true);
            }
            break;
        case 4:  // Trade cancelled
            if (player_relations.hasPermission(tradePartnerName,
                PlayerRelation::SPEECH_LOG))
            {
                NotifyManager::notify(NotifyTypes::TRADE_CANCELLED_NAME,
                    tradePartnerName);
            }
            // otherwise ignore silently

            if (tradeWindow)
            {
                tradeWindow->setVisible(false);
//                        tradeWindow->clear();
            }
            PlayerInfo::setTrading(false);
            break;
        case 5:
            NotifyManager::notify(NotifyTypes::TRADE_CANCELLED_BUSY,
                tradePartnerName);
            break;
        default:  // Shouldn't happen as well, but to be sure
            NotifyManager::notify(NotifyTypes::TRADE_ERROR_UNKNOWN,
                tradePartnerName);
            if (tradeWindow)
                tradeWindow->clear();
            break;
    }
}

void TradeHandler::processTradeOk(Net::MessageIn &msg)
{
    // 0 means ok from myself, 1 means ok from other;
    if (tradeWindow)
        tradeWindow->receivedOk(msg.readUInt8("status") == 0U);
    else
        msg.readUInt8("status");
}

void TradeHandler::processTradeCancel(Net::MessageIn &msg A_UNUSED)
{
    NotifyManager::notify(NotifyTypes::TRADE_CANCELLED);
    if (tradeWindow)
    {
        tradeWindow->setVisible(false);
        tradeWindow->reset();
    }
    PlayerInfo::setTrading(false);
}

void TradeHandler::processTradeComplete(Net::MessageIn &msg A_UNUSED)
{
    NotifyManager::notify(NotifyTypes::TRADE_COMPLETE);
    if (tradeWindow)
    {
        tradeWindow->setVisible(false);
        tradeWindow->reset();
    }
    PlayerInfo::setTrading(false);
}

void TradeHandler::processTradeRequestContinue(const std::string &partner)
{
    if (player_relations.hasPermission(partner,
        PlayerRelation::TRADE))
    {
        if (PlayerInfo::isTrading() || confirmDlg)
        {
            tradeHandler->respond(false);
            return;
        }

        tradePartnerName = partner;
        PlayerInfo::setTrading(true);
        if (tradeWindow)
        {
            if (tradePartnerName.empty() || tradeWindow->getAutoTradeNick()
                != tradePartnerName)
            {
                tradeWindow->clear();
                // TRANSLATORS: trade message
                confirmDlg = new ConfirmDialog(_("Request for Trade"),
                    // TRANSLATORS: trade message
                    strprintf(_("%s wants to trade with you, do"
                    " you accept?"), tradePartnerName.c_str()),
                    SOUND_REQUEST, true);
                confirmDlg->postInit();
                confirmDlg->addActionListener(&listener);
            }
            else
            {
                tradeHandler->respond(true);
            }
        }
    }
    else
    {
        tradeHandler->respond(false);
        return;
    }
}

}  // namespace Ea
