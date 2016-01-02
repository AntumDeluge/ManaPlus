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

#ifndef NET_TMWA_GENERALHANDLER_H
#define NET_TMWA_GENERALHANDLER_H

#include "net/generalhandler.h"

namespace TmwAthena
{
class AdminHandler;
class BeingHandler;
class BuySellHandler;
class CharServerHandler;
class ChatHandler;
class GameHandler;
class GuildHandler;
class InventoryHandler;
class ItemHandler;
class LoginHandler;
class NpcHandler;
class PartyHandler;
class PetHandler;
class PlayerHandler;
class SkillHandler;
class TradeHandler;
class QuestHandler;
class ServerFeatures;
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
class RouletteHandler;
class SearchStoreHandler;
class VendingHandler;
#endif

class GeneralHandler final : public Net::GeneralHandler
{
    public:
        GeneralHandler();

        A_DELETE_COPY(GeneralHandler)

        ~GeneralHandler();

        void load() override final;

        void reload() override final;

        void unload() override final;

        void flushNetwork() override final;

        void flushSend() override final;

        void clearHandlers() override final;

        void reloadPartially() const override final;

        void gameStarted() const override final;

        void gameEnded() const override final;

    protected:
        AdminHandler *mAdminHandler;
        BeingHandler *mBeingHandler;
        BuySellHandler *mBuySellHandler;
        CharServerHandler *mCharServerHandler;
        ChatHandler *mChatHandler;
        GameHandler *mGameHandler;
        GuildHandler *mGuildHandler;
        InventoryHandler *mInventoryHandler;
        ItemHandler *mItemHandler;
        LoginHandler *mLoginHandler;
        NpcHandler *mNpcHandler;
        PartyHandler *mPartyHandler;
        PetHandler *mPetHandler;
        PlayerHandler *mPlayerHandler;
        SkillHandler *mSkillHandler;
        TradeHandler *mTradeHandler;
        QuestHandler *mQuestHandler;
        ServerFeatures *mServerFeatures;
#ifdef EATHENA_SUPPORT
        AuctionHandler *mAuctionHandler;
        BankHandler *mBankHandler;
        BattleGroundHandler *mBattleGroundHandler;
        BuyingStoreHandler *mBuyingStoreHandler;
        CashShopHandler *mCashShopHandler;
        ElementalHandler *mElementalHandler;
        FamilyHandler *mFamilyHandler;
        FriendsHandler *mFriendsHandler;
        HomunculusHandler *mHomunculusHandler;
        MailHandler *mMailHandler;
        MapHandler *mMapHandler;
        MarketHandler *mMarketHandler;
        MercenaryHandler *mMercenaryHandler;
        RouletteHandler *mRouletteHandler;
        SearchStoreHandler *mSearchStoreHandler;
        VendingHandler *mVendingHandler;
#endif
};

}  // namespace TmwAthena

#endif  // NET_TMWA_GENERALHANDLER_H
