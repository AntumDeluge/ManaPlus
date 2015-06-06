/*
 *  The ManaPlus Client
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef NET_BEINGHANDLER_H
#define NET_BEINGHANDLER_H

#include "being/being.h"

#include "enums/being/rank.h"

#include "net/messagein.h"

namespace Net
{

class BeingHandler notfinal
{
    public:
        virtual ~BeingHandler()
        { }

        virtual void handleMessage(Net::MessageIn &msg) = 0;

        virtual void requestNameById(const BeingId id) const = 0;

        virtual void undress(Being *const being) const = 0;

#ifdef EATHENA_SUPPORT
        virtual void requestRanks(const RankT rank) const = 0;
#endif
};

}  // namespace Net

extern Net::BeingHandler *beingHandler;

#endif  // NET_BEINGHANDLER_H
