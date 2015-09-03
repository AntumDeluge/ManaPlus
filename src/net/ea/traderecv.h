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

#ifndef NET_EA_TRADERECV_H
#define NET_EA_TRADERECV_H

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
#include <cstdint>
#else
#include <stdint.h>
#endif

#include <string>

namespace Net
{
    class MessageIn;
}

namespace Ea
{
    namespace TradeRecv
    {
        void processTradeOk(Net::MessageIn &msg);
        void processTradeCancel(Net::MessageIn &msg);
        void processTradeComplete(Net::MessageIn &msg);
        void processTradeRequestContinue(const std::string &partner);
        void processTradeResponseContinue(const uint8_t type);
    }  // namespace TradeRecv
}  // namespace Ea

#endif  // NET_EA_TRADERECV_H
