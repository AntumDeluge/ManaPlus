/*
 *  The ManaPlus Client
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

#include "net/tmwa/guildrecv.h"

#include "configuration.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "gui/windows/chatwindow.h"
#include "gui/windows/socialwindow.h"

#include "net/serverfeatures.h"

#include "net/messagein.h"

#include "net/tmwa/guildhandler.h"

#include "gui/widgets/tabs/chat/guildtab.h"

#include "net/ea/guildrecv.h"

#include "debug.h"

namespace TmwAthena
{

void GuildRecv::processGuildPositionInfo(Net::MessageIn &msg)
{
    const int guildId = msg.readInt32("guild id");
    const int emblem  = msg.readInt32("emblem");
    const int posMode = msg.readInt32("position");
    msg.readInt32("unused");
    msg.readUInt8("usused");
    std::string guildName = msg.readString(24, "guild name");

    PlayerInfo::setGuildPositionFlags(GuildPositionFlags::Invite);

    Guild *const g = Guild::getGuild(static_cast<int16_t>(guildId));
    if (!g)
        return;

    g->setName(guildName);
    g->setEmblemId(emblem);
    if (!Ea::taGuild)
        Ea::taGuild = g;
    if (!guildTab && chatWindow)
    {
        guildTab = new GuildTab(chatWindow);
        if (config.getBoolValue("showChatHistory"))
            guildTab->loadFromLogFile("#Guild");
        if (localPlayer)
            localPlayer->addGuild(Ea::taGuild);
        guildHandler->memberList();
    }

    if (localPlayer)
    {
        localPlayer->setGuild(g);
        localPlayer->setGuildName(g->getName());
    }

    logger->log("Guild position info: %d %d %d %s\n", guildId,
                emblem, posMode, guildName.c_str());
}

void GuildRecv::processGuildMemberLogin(Net::MessageIn &msg)
{
    const BeingId accountId = msg.readBeingId("account id");
    const int charId = msg.readInt32("char id");
    const int online = msg.readInt32("flag");
    if (Ea::taGuild)
    {
        GuildMember *const m = Ea::taGuild->getMember(accountId, charId);
        if (m)
        {
            m->setOnline(online);
            if (guildTab)
                guildTab->showOnline(m->getName(), fromBool(online, Online));
            if (socialWindow)
                socialWindow->updateGuildCounter();
        }
    }
}

void GuildRecv::processGuildExpulsion(Net::MessageIn &msg)
{
    std::string nick;
    if (serverFeatures->haveNewGuild())
    {
        nick = msg.readString(24, "name");
        msg.readString(40, "reason");
        msg.readString(24, "kicker name");
    }
    else
    {
        msg.readInt16("len?");
        nick = msg.readString(24, "name?");
        msg.skip(24, "player name");
        msg.readString(44, "message");
    }
    Ea::GuildRecv::processGuildExpulsionContinue(nick);
}

void GuildRecv::processGuildExpulsionList(Net::MessageIn &msg)
{
    const int length = msg.readInt16("len");
    if (length < 4)
        return;

    const int count = (length - 4) / 88;

    for (int i = 0; i < count; i++)
    {
        msg.readString(24, "name of expulsed");
        msg.readString(24, "name of expluser");
        msg.readString(24, "message");
    }
}

}  // namespace TmwAthena
