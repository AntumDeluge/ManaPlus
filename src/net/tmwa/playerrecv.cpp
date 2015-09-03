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

#include "net/tmwa/playerrecv.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "gui/windows/statuswindow.h"

#include "net/messagein.h"

#include "debug.h"

namespace TmwAthena
{

void PlayerRecv::processPlayerStatUpdate5(Net::MessageIn &msg)
{
    BLOCK_START("PlayerRecv::processPlayerStatUpdate5")
    PlayerInfo::setAttribute(Attributes::CHAR_POINTS,
        msg.readInt16("char points"));

    unsigned int val = msg.readUInt8("str");
    PlayerInfo::setStatBase(Attributes::STR, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::STR,
            msg.readUInt8("str cost"));
    }
    else
    {
        msg.readUInt8("str cost");
    }

    val = msg.readUInt8("agi");
    PlayerInfo::setStatBase(Attributes::AGI, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::AGI,
            msg.readUInt8("agi cost"));
    }
    else
    {
        msg.readUInt8("agi cost");
    }

    val = msg.readUInt8("vit");
    PlayerInfo::setStatBase(Attributes::VIT, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::VIT,
            msg.readUInt8("vit cost"));
    }
    else
    {
        msg.readUInt8("vit cost");
    }

    val = msg.readUInt8("int");
    PlayerInfo::setStatBase(Attributes::INT, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::INT,
            msg.readUInt8("int cost"));
    }
    else
    {
        msg.readUInt8("int cost");
    }

    val = msg.readUInt8("dex");
    PlayerInfo::setStatBase(Attributes::DEX, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::DEX,
            msg.readUInt8("dex cost"));
    }
    else
    {
        msg.readUInt8("dex cost");
    }

    val = msg.readUInt8("luk");
    PlayerInfo::setStatBase(Attributes::LUK, val);
    if (statusWindow)
    {
        statusWindow->setPointsNeeded(Attributes::LUK,
            msg.readUInt8("luk cost"));
    }
    else
    {
        msg.readUInt8("luk cost");
    }

    PlayerInfo::setStatBase(Attributes::ATK,
        msg.readInt16("atk"), Notify_false);
    PlayerInfo::setStatMod(Attributes::ATK, msg.readInt16("atk+"));
    PlayerInfo::updateAttrs();

    val = msg.readInt16("matk");
    PlayerInfo::setStatBase(Attributes::MATK, val, Notify_false);

    val = msg.readInt16("matk+");
    PlayerInfo::setStatMod(Attributes::MATK, val);

    PlayerInfo::setStatBase(Attributes::DEF,
        msg.readInt16("def"), Notify_false);
    PlayerInfo::setStatMod(Attributes::DEF, msg.readInt16("def+"));

    PlayerInfo::setStatBase(Attributes::MDEF,
        msg.readInt16("mdef"), Notify_false);
    PlayerInfo::setStatMod(Attributes::MDEF, msg.readInt16("mdef+"));

    PlayerInfo::setStatBase(Attributes::HIT, msg.readInt16("hit"));

    PlayerInfo::setStatBase(Attributes::FLEE,
        msg.readInt16("flee"), Notify_false);
    PlayerInfo::setStatMod(Attributes::FLEE, msg.readInt16("flee+"));

    PlayerInfo::setStatBase(Attributes::CRIT, msg.readInt16("crit"));

    PlayerInfo::setStatBase(Attributes::MANNER, msg.readInt16("manner"));
    msg.readInt16("unused?");
    BLOCK_END("PlayerRecv::processPlayerStatUpdate5")
}

void PlayerRecv::processWalkResponse(Net::MessageIn &msg)
{
    BLOCK_START("PlayerRecv::processWalkResponse")
    /*
      * This client assumes that all walk messages succeed,
      * and that the server will send a correction notice
      * otherwise.
      */
    uint16_t srcX, srcY, dstX, dstY;
    msg.readInt32("tick");
    msg.readCoordinatePair(srcX, srcY, dstX, dstY, "move path");
    msg.readUInt8("unused");
    if (localPlayer)
        localPlayer->setRealPos(dstX, dstY);
    BLOCK_END("PlayerRecv::processWalkResponse")
}

}  // namespace TmwAthena
