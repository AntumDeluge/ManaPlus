/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "main.h"

#include "net/net.h"

#include "configuration.h"

#include "net/loginhandler.h"

#ifdef TMWA_SUPPORT
#include "net/tmwa/generalhandler.h"
#endif

#ifdef EATHENA_SUPPORT
#include "net/eathena/generalhandler.h"
#endif

#include "debug.h"

namespace Net
{
#ifdef EATHENA_SUPPORT
    class AuctionHandler;
    class BankHandler;
    class BattleGroundHandler;
    class BuyingStoreHandler;
    class CashShopHandler;
    class ElementalHandler;
    class FamilyHandler;
    class FriendsHandler;
    class HomunculusHandler;
    class MailHandler;
    class MapHandler;
    class MarketHandler;
    class MercenaryHandler;
    class SearchStoreHandler;
    class RouletteHandler;
    class VendingHandler;
#endif
    class AdminHandler;
    class BeingHandler;
    class BuySellHandler;
    class CharServerHandler;
    class ChatHandler;
    class GameHandler;
    class GuildHandler;
    class InventoryHandler;
    class NpcHandler;
    class PartyHandler;
    class PetHandler;
    class PlayerHandler;
    class QuestHandler;
    class ServerFeatures;
    class SkillHandler;
    class TradeHandler;
}  // namespace Net

Net::AdminHandler *adminHandler = nullptr;
Net::CharServerHandler *charServerHandler = nullptr;
Net::ChatHandler *chatHandler = nullptr;
Net::GeneralHandler *generalHandler = nullptr;
Net::InventoryHandler *inventoryHandler = nullptr;
Net::LoginHandler *loginHandler = nullptr;
Net::GameHandler *gameHandler = nullptr;
Net::GuildHandler *guildHandler = nullptr;
Net::NpcHandler *npcHandler = nullptr;
Net::PartyHandler *partyHandler = nullptr;
Net::PetHandler *petHandler = nullptr;
Net::PlayerHandler *playerHandler = nullptr;
Net::SkillHandler *skillHandler = nullptr;
Net::TradeHandler *tradeHandler = nullptr;
Net::BeingHandler *beingHandler = nullptr;
Net::BuySellHandler *buySellHandler = nullptr;
Net::ServerFeatures *serverFeatures = nullptr;
#ifdef EATHENA_SUPPORT
Net::AuctionHandler *auctionHandler = nullptr;
Net::BankHandler *bankHandler = nullptr;
Net::BattleGroundHandler *battleGroundHandler = nullptr;
Net::BuyingStoreHandler *buyingStoreHandler = nullptr;
Net::CashShopHandler *cashShopHandler = nullptr;
Net::ElementalHandler *elementalHandler = nullptr;
Net::FamilyHandler *familyHandler = nullptr;
Net::FriendsHandler *friendsHandler = nullptr;
Net::HomunculusHandler *homunculusHandler = nullptr;
Net::MailHandler *mailHandler = nullptr;
Net::MapHandler *mapHandler = nullptr;
Net::MarketHandler *marketHandler = nullptr;
Net::MercenaryHandler *mercenaryHandler = nullptr;
Net::RouletteHandler *rouletteHandler = nullptr;
Net::SearchStoreHandler *searchStoreHandler = nullptr;
Net::VendingHandler *vendingHandler = nullptr;
#endif
Net::QuestHandler *questHandler = nullptr;

namespace Net
{
ServerTypeT networkType = ServerType::UNKNOWN;
std::set<int> ignorePackets;

void connectToServer(const ServerInfo &server)
{
    BLOCK_START("Net::connectToServer")
    if (networkType == server.type && generalHandler)
    {
        generalHandler->reload();
    }
    else
    {
        if (networkType != ServerType::UNKNOWN && generalHandler)
            generalHandler->unload();

        switch (server.type)
        {
            case ServerType::EATHENA:
            case ServerType::EVOL2:
#ifdef EATHENA_SUPPORT
                new EAthena::GeneralHandler;
#else
                new TmwAthena::GeneralHandler;
#endif
                break;
            case ServerType::TMWATHENA:
            case ServerType::EVOL:
            case ServerType::UNKNOWN:
            default:
#ifdef TMWA_SUPPORT
                new TmwAthena::GeneralHandler;
#else
                new EAthena::GeneralHandler;
#endif
                break;
        }

        generalHandler->load();

        networkType = server.type;
    }

    if (loginHandler)
    {
        loginHandler->setServer(server);
        loginHandler->connect();
    }
    BLOCK_END("Net::connectToServer")
}

void unload()
{
    GeneralHandler *const handler = generalHandler;
    if (handler)
        handler->unload();
    ignorePackets.clear();
}

ServerTypeT getNetworkType()
{
    return networkType;
}

void loadIgnorePackets()
{
    const std::string str = config.getStringValue("ignorelogpackets");
    splitToIntSet(ignorePackets, str, ',');
}

bool isIgnorePacket(const int id)
{
    return ignorePackets.find(id) != ignorePackets.end();
}

}  // namespace Net
