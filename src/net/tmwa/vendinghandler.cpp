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

#include "net/tmwa/vendinghandler.h"

#include "debug.h"

extern Net::VendingHandler *vendingHandler;

namespace TmwAthena
{

VendingHandler::VendingHandler()
{
    vendingHandler = this;
}

void VendingHandler::close() const
{
}

void VendingHandler::open(const Being *const being A_UNUSED) const
{
}

void VendingHandler::buy(const Being *const being A_UNUSED,
                         const int index A_UNUSED,
                         const int amount A_UNUSED) const
{
}

void VendingHandler::buy2(const Being *const being A_UNUSED,
                          const int vendId A_UNUSED,
                          const int index A_UNUSED,
                          const int amount A_UNUSED) const
{
}

void VendingHandler::createShop(const std::string &name A_UNUSED,
                                const bool flag A_UNUSED,
                                std::vector<ShopItem*> &items A_UNUSED) const
{
}

}  // namespace TmwAthena
