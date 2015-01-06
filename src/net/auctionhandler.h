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

#ifndef NET_AUCTIONHANDLER_H
#define NET_AUCTIONHANDLER_H

#include <string>

#include "localconsts.h"

namespace Net
{

class AuctionHandler notfinal
{
    public:
        virtual ~AuctionHandler()
        { }

        virtual void cancelRequest() const = 0;
};

}  // namespace Net

extern Net::AuctionHandler *auctionHandler;

#endif  // NET_AUCTIONHANDLER_H
