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

#ifndef NET_MERCENARYHANDLER_H
#define NET_MERCENARYHANDLER_H

#ifdef EATHENA_SUPPORT

#include "enums/simpletypes/beingid.h"
#include "enums/simpletypes/keep.h"

#include <string>

#include "localconsts.h"

namespace Net
{

class MercenaryHandler notfinal
{
    public:
        virtual ~MercenaryHandler()
        { }

        virtual void handleMercenaryMessage(const int cmd) = 0;

        virtual void fire() = 0;

        virtual void moveToMaster() const = 0;

        virtual void move(const int x, const int y) const = 0;

        virtual void attack(const BeingId targetId,
                            const Keep keep) const = 0;

        virtual void talk(const std::string &restrict text) const = 0;

        virtual void emote(const uint8_t emoteId) const = 0;

        virtual void setDirection(const unsigned char type) const = 0;
};

}  // namespace Net

extern Net::MercenaryHandler *mercenaryHandler;

#endif  // EATHENA_SUPPORT
#endif  // NET_MERCENARYHANDLER_H
