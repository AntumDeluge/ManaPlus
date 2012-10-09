/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#ifndef NET_EA_PLAYERHANDLER_H
#define NET_EA_PLAYERHANDLER_H

#include "net/messagein.h"
#include "net/net.h"
#include "net/playerhandler.h"

namespace Ea
{

class PlayerHandler : public Net::PlayerHandler
{
    public:
        PlayerHandler();

        A_DELETE_COPY(PlayerHandler);

        void decreaseAttribute(int attr);

        void ignorePlayer(const std::string &player, bool ignore);

        void ignoreAll(bool ignore);

        bool canUseMagic() const;

        bool canCorrectAttributes() const;

        Vector getDefaultWalkSpeed() const;

        PlayerInfo::Attribute getJobLocation() const;

        PlayerInfo::Attribute getAttackLocation() const;

        void processWalkResponse(Net::MessageIn &msg);

        void processPlayerWarp(Net::MessageIn &msg);

        void processPlayerStatUpdate1(Net::MessageIn &msg);

        void processPlayerStatUpdate2(Net::MessageIn &msg);

        void processPlayerStatUpdate3(Net::MessageIn &msg);

        void processPlayerStatUpdate4(Net::MessageIn &msg);

        void processPlayerStatUpdate5(Net::MessageIn &msg);

        void processPlayerStatUpdate6(Net::MessageIn &msg);

        void processPlayerArrowMessage(Net::MessageIn &msg);
};

} // namespace Ea

#endif // NET_EA_PLAYERHANDLER_H
