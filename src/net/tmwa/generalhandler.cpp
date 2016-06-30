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

#include "net/tmwa/generalhandler.h"

#include "client.h"
#include "configuration.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/socialwindow.h"
#include "gui/windows/statuswindow.h"

#include "gui/widgets/tabs/chat/guildtab.h"
#include "gui/widgets/tabs/chat/partytab.h"

#include "net/tmwa/adminhandler.h"
#include "net/tmwa/beinghandler.h"
#include "net/tmwa/buysellhandler.h"
#include "net/tmwa/chathandler.h"
#include "net/tmwa/charserverhandler.h"
#include "net/tmwa/gamehandler.h"
#include "net/tmwa/guildhandler.h"
#include "net/tmwa/inventoryhandler.h"
#include "net/tmwa/itemhandler.h"
#include "net/tmwa/loginhandler.h"
#include "net/tmwa/network.h"
#include "net/tmwa/npchandler.h"
#include "net/tmwa/partyhandler.h"
#include "net/tmwa/pethandler.h"
#include "net/tmwa/playerhandler.h"
#include "net/tmwa/serverfeatures.h"
#include "net/tmwa/tradehandler.h"
#include "net/tmwa/skillhandler.h"
#include "net/tmwa/questhandler.h"

#ifdef EATHENA_SUPPORT
#include "net/tmwa/auctionhandler.h"
#include "net/tmwa/bankhandler.h"
#include "net/tmwa/battlegroundhandler.h"
#include "net/tmwa/buyingstorehandler.h"
#include "net/tmwa/cashshophandler.h"
#include "net/tmwa/elementalhandler.h"
#include "net/tmwa/familyhandler.h"
#include "net/tmwa/friendshandler.h"
#include "net/tmwa/homunculushandler.h"
#include "net/tmwa/mailhandler.h"
#include "net/tmwa/maphandler.h"
#include "net/tmwa/markethandler.h"
#include "net/tmwa/mercenaryhandler.h"
#include "net/tmwa/roulettehandler.h"
#include "net/tmwa/searchstorehandler.h"
#include "net/tmwa/vendinghandler.h"
#endif

#include "utils/delete2.h"
#include "utils/gettext.h"

#include "debug.h"

extern Net::GeneralHandler *generalHandler;

namespace TmwAthena
{

GeneralHandler::GeneralHandler() :
    mAdminHandler(new AdminHandler),
    mBeingHandler(new BeingHandler(config.getBoolValue("EnableSync"))),
    mBuySellHandler(new BuySellHandler),
    mCharServerHandler(new CharServerHandler),
    mChatHandler(new ChatHandler),
    mGameHandler(new GameHandler),
    mGuildHandler(new GuildHandler),
    mInventoryHandler(new InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new LoginHandler),
    mNpcHandler(new NpcHandler),
    mPartyHandler(new PartyHandler),
    mPetHandler(new PetHandler),
    mPlayerHandler(new PlayerHandler),
    mSkillHandler(new SkillHandler),
    mTradeHandler(new TradeHandler),
    mQuestHandler(new QuestHandler),
#ifdef EATHENA_SUPPORT
    mServerFeatures(new ServerFeatures),
    mAuctionHandler(new AuctionHandler),
    mBankHandler(new BankHandler),
    mBattleGroundHandler(new BattleGroundHandler),
    mBuyingStoreHandler(new BuyingStoreHandler),
    mCashShopHandler(new CashShopHandler),
    mElementalHandler(new ElementalHandler),
    mFamilyHandler(new FamilyHandler),
    mFriendsHandler(new FriendsHandler),
    mHomunculusHandler(new HomunculusHandler),
    mMailHandler(new MailHandler),
    mMapHandler(new MapHandler),
    mMarketHandler(new MarketHandler),
    mMercenaryHandler(new MercenaryHandler),
    mRouletteHandler(new RouletteHandler),
    mSearchStoreHandler(new SearchStoreHandler),
    mVendingHandler(new VendingHandler)
#else
    mServerFeatures(new ServerFeatures)
#endif
{
    generalHandler = this;
}

GeneralHandler::~GeneralHandler()
{
    delete2(Network::mInstance);
}

void GeneralHandler::load() const
{
    new Network;
    Network::mInstance->registerHandlers();
}

void GeneralHandler::reload() const
{
    if (Network::mInstance)
        Network::mInstance->disconnect();

    static_cast<LoginHandler*>(mLoginHandler)->clearWorlds();
    const CharServerHandler *const charHandler =
        static_cast<CharServerHandler*>(mCharServerHandler);
    charHandler->setCharCreateDialog(nullptr);
    charHandler->setCharSelectDialog(nullptr);
    static_cast<PartyHandler*>(mPartyHandler)->reload();
}

void GeneralHandler::reloadPartially() const
{
    static_cast<PartyHandler*>(mPartyHandler)->reload();
}

void GeneralHandler::unload() const
{
    clearHandlers();
}

void GeneralHandler::flushSend() const
{
    if (!Network::mInstance)
        return;

    Network::mInstance->flush();
}

void GeneralHandler::flushNetwork() const
{
    if (!Network::mInstance)
        return;

    BLOCK_START("GeneralHandler::flushNetwork 1")
    Network::mInstance->flush();
    BLOCK_END("GeneralHandler::flushNetwork 1")
    Network::mInstance->dispatchMessages();

    BLOCK_START("GeneralHandler::flushNetwork 3")
    if (Network::mInstance->getState() == Network::NET_ERROR)
    {
        if (!Network::mInstance->getError().empty())
        {
            errorMessage = Network::mInstance->getError();
        }
        else
        {
            // TRANSLATORS: error message
            errorMessage = _("Got disconnected from server!");
        }

        client->setState(State::ERROR);
    }
    BLOCK_END("GeneralHandler::flushNetwork 3")
}

void GeneralHandler::clearHandlers() const
{
    if (Network::mInstance)
        Network::mInstance->clearHandlers();
}

void GeneralHandler::gameStarted() const
{
    if (skillDialog)
        skillDialog->loadSkills();

    if (!statusWindow)
        return;

    statusWindow->addAttributes();
}

void GeneralHandler::gameEnded() const
{
    if (socialWindow)
    {
        socialWindow->removeTab(Ea::taGuild);
        socialWindow->removeTab(Ea::taParty);
    }

    delete2(guildTab);
    delete2(partyTab);
}

}  // namespace TmwAthena
