/*
 *  The ManaPlus Client
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef NET_EA_GUILDRECV_H
#define NET_EA_GUILDRECV_H

#include <string>

namespace Net
{
    class MessageIn;
}

class Guild;

namespace Ea
{
    namespace GuildRecv
    {
        extern bool showBasicInfo;

        void processGuildCreateResponse(Net::MessageIn &msg);
        void processGuildMasterOrMember(Net::MessageIn &msg);
        void processGuildBasicInfo(Net::MessageIn &msg);
        void processGuildAlianceInfo(Net::MessageIn &msg);
        void processGuildMemberList(Net::MessageIn &msg);
        void processGuildPosNameList(Net::MessageIn &msg);
        void processGuildPosInfoList(Net::MessageIn &msg);
        void processGuildPositionChanged(Net::MessageIn &msg);
        void processGuildMemberPosChange(Net::MessageIn &msg);
        void processGuildEmblemData(Net::MessageIn &msg);
        void processGuildSkillInfo(Net::MessageIn &msg);
        void processGuildNotice(Net::MessageIn &msg);
        void processGuildInvite(Net::MessageIn &msg);
        void processGuildInviteAck(Net::MessageIn &msg);
        void processGuildLeave(Net::MessageIn &msg);
        void processGuildMessage(Net::MessageIn &msg);
        void processGuildSkillUp(Net::MessageIn &msg);
        void processGuildReqAlliance(Net::MessageIn &msg);
        void processGuildReqAllianceAck(Net::MessageIn &msg);
        void processGuildDelAlliance(Net::MessageIn &msg);
        void processGuildOppositionAck(Net::MessageIn &msg);
        void processGuildBroken(Net::MessageIn &msg);
        void processGuildExpulsionContinue(const std::string &nick);
    }  // namespace GuildRecv

    extern Guild *taGuild;
}  // namespace Ea

#endif  // NET_EA_GUILDRECV_H
