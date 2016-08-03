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

#include "net/eathena/elementalrecv.h"

#include "being/playerinfo.h"

#include "net/messagein.h"

#include "net/eathena/sp.h"

#include "utils/checkutils.h"

#include "debug.h"

namespace EAthena
{

#define setElementalStat(sp, stat) \
        case sp: \
            PlayerInfo::setStatBase(stat, \
                val); \
            break;

void ElementalRecv::processElementalUpdateStatus(Net::MessageIn &msg)
{
    const int sp = msg.readInt16("type");
    const int val = msg.readInt32("value");
    switch (sp)
    {
        setElementalStat(Sp::HP, Attributes::ELEMENTAL_HP);
        setElementalStat(Sp::MAXHP, Attributes::ELEMENTAL_MAX_HP);
        setElementalStat(Sp::SP, Attributes::ELEMENTAL_MP);
        setElementalStat(Sp::MAXSP, Attributes::ELEMENTAL_MAX_MP);
        default:
            reportAlways("Unknown elemental stat %d",
                sp);
    }
}

void ElementalRecv::processElementalInfo(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("elemental id");
    PlayerInfo::setStatBase(Attributes::ELEMENTAL_HP,
        msg.readInt32("hp"));
    PlayerInfo::setStatBase(Attributes::ELEMENTAL_MAX_HP,
        msg.readInt32("max hp"));
    PlayerInfo::setStatBase(Attributes::ELEMENTAL_MP,
        msg.readInt32("sp"));
    PlayerInfo::setStatBase(Attributes::ELEMENTAL_MAX_MP,
        msg.readInt32("max sp"));
    PlayerInfo::setElemental(id);
}

}  // namespace EAthena
