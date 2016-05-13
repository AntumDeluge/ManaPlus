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

#include "net/eathena/auctionrecv.h"

#include "logger.h"

#include "const/resources/item/cards.h"

#include "net/messagein.h"

#include "debug.h"

namespace EAthena
{

void AuctionRecv::processOpenWindow(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt32("flag");  // 0 - open, 1 - close
}

void AuctionRecv::processAuctionResults(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("len");
    msg.readInt32("pages");
    const int itemCount = msg.readInt32("items count");
    for (int f = 0; f < itemCount; f ++)
    {
        msg.readInt32("auction id");
        msg.readString(24, "seller name");
        msg.readInt32("item id");
        msg.readInt32("auction type");
        msg.readInt16("item amount");  // always 1
        msg.readUInt8("identify");
        msg.readUInt8("attribute");
        msg.readUInt8("refine");
        for (int d = 0; d < maxCards; d ++)
            msg.readUInt16("card");
        msg.readInt32("price");
        msg.readInt32("buy now");
        msg.readString(24, "buyer name");
        msg.readInt32("timestamp");
        // +++ need use ItemColorManager for color
    }
}

void AuctionRecv::processAuctionSetItem(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("index");
    msg.readUInt8("flag");
}

void AuctionRecv::processAuctionMessage(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readUInt8("message");
}

void AuctionRecv::processAuctionClose(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("flag");
}

}  // namespace EAthena
