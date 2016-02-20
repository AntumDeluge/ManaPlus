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

#include "net/eathena/familyhandler.h"

#include "being/being.h"

#include "net/eathena/familyrecv.h"
#include "net/eathena/messageout.h"
#include "net/eathena/protocolout.h"

#include "debug.h"

extern Net::FamilyHandler *familyHandler;

namespace EAthena
{

FamilyHandler::FamilyHandler()
{
    FamilyRecv::mParent1 = 0;
    FamilyRecv::mParent2 = 0;

    familyHandler = this;
}

void FamilyHandler::askForChild(const Being *const being) const
{
    if (!being)
        return;

    createOutPacket(CMSG_FAMILY_ASK_FOR_CHILD);
    outMsg.writeBeingId(being->getId(), "account id");
}

void FamilyHandler::askForChildReply(const bool accept) const
{
    createOutPacket(CMSG_FAMILY_ASK_FOR_CHILD_REPLY);
    outMsg.writeInt32(FamilyRecv::mParent1, "parent1");
    outMsg.writeInt32(FamilyRecv::mParent2, "parent2");
    outMsg.writeInt32(accept ? 0: 1, "result");
}

}  // namespace EAthena
