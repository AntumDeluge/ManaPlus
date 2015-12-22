/*
 *  The ManaPlus Client
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

#ifndef NET_TMWA_MARKETHANDLER_H
#define NET_TMWA_MARKETHANDLER_H

#ifdef EATHENA_SUPPORT

#include "net/markethandler.h"

namespace TmwAthena
{
class MarketHandler final : public Net::MarketHandler
{
    public:
        MarketHandler();

        A_DELETE_COPY(MarketHandler)

        void close() override final A_CONST;

        void buyItem(const int itemId,
                     const int type,
                     const ItemColor color,
                     const int amount) const override final A_CONST;

        void buyItems(std::vector<ShopItem*> &items) const override final
                      A_CONST;
};

}  // namespace TmwAthena

#endif  // EATHENA_SUPPORT
#endif  // NET_TMWA_MARKETHANDLER_H
