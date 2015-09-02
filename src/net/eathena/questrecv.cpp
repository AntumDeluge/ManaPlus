/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2015  The ManaPlus Developers
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

#include "net/eathena/questrecv.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/questswindow.h"

#include "net/messagein.h"

#include "net/eathena/messageout.h"
#include "net/eathena/protocol.h"

#include "resources/skillconsts.h"

#include "debug.h"

namespace EAthena
{

void QuestRecv::processAddQuest(Net::MessageIn &msg)
{
    const int var = msg.readInt32("quest id");
    const int val = msg.readUInt8("state");
    msg.readUInt8("time diff");
    msg.readInt32("time");
    const int num = msg.readInt16("objectives count");
    for (int f = 0; f < num; f ++)
    {
        // need use in quests kills list
        msg.readInt32("monster id");
        msg.readInt16("count");
        msg.readString(24, "monster name");
    }

    msg.skipToEnd("unused");

    if (questsWindow)
    {
        questsWindow->updateQuest(var, val);
        questsWindow->rebuild(true);
    }
    if (skillDialog)
    {
        skillDialog->updateQuest(var, val);
        skillDialog->playUpdateEffect(var + SKILL_VAR_MIN_ID);
    }
}

void QuestRecv::processAddQuests(Net::MessageIn &msg)
{
    msg.readInt16("len");
    const int num = msg.readInt32("quests count");
    for (int f = 0; f < num; f ++)
    {
        const int var = msg.readInt32("quest id");
        const int val = msg.readUInt8("state");
        msg.readInt32("time diff");
        msg.readInt32("time");
        msg.readInt16("objectives count");
        if (questsWindow)
            questsWindow->updateQuest(var, val);
        if (skillDialog)
            skillDialog->updateQuest(var, val);
    }

    if (questsWindow)
        questsWindow->rebuild(false);
}

void QuestRecv::processAddQuestsObjectives(Net::MessageIn &msg)
{
    msg.readInt16("len");
    const int quests = msg.readInt32("quests count");
    for (int f = 0; f < quests; f ++)
    {
        msg.readInt32("quest id");
        msg.readInt32("time diff");
        msg.readInt32("time");
        const int num = msg.readInt16("objectives count");
        for (int d = 0; d < num; d ++)
        {
            // need use in quests kills list
            msg.readInt32("monster id");
            msg.readInt16("count");
            msg.readString(24, "monster name");
        }
    }
    msg.skipToEnd("unused");
}

void QuestRecv::processUpdateQuestsObjectives(Net::MessageIn &msg)
{
    // ignored
    msg.readInt16("len");
    const int num = msg.readInt16("objectives count");
    for (int f = 0; f < num; f ++)
    {
        msg.readInt32("quest id");
        msg.readInt32("monster id");
        msg.readInt16("count old");
        msg.readInt16("count new");
    }
}

void QuestRecv::processRemoveQuest(Net::MessageIn &msg)
{
    const int var = msg.readInt32("quest id");
    const int val = -1;

    // not removing quest, because this is impossible,
    // but changing status to -1
    if (questsWindow)
    {
        questsWindow->updateQuest(var, val);
        questsWindow->rebuild(true);
    }
    if (skillDialog)
    {
        skillDialog->updateQuest(var, val);
        skillDialog->playUpdateEffect(var + SKILL_VAR_MIN_ID);
    }
}

void QuestRecv::processActivateQuest(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ need enable/disable quests depend on this packet
    msg.readInt32("quest id");
    msg.readUInt8("activate");
}

void QuestRecv::processNpcQuestEffect(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // this packed mostly useless, because manaplus can show any
    // kind of effects based on quest states.
    msg.readInt32("npc id");
    msg.readInt16("x");
    msg.readInt16("y");
    msg.readInt16("state");
    msg.readInt16("color");
}

}  // namespace EAthena
