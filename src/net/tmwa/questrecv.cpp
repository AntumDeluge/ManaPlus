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

#include "net/tmwa/questrecv.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/questswindow.h"

#include "net/messagein.h"

#include "const/resources/skill.h"

#include "debug.h"

namespace TmwAthena
{

void QuestRecv::processSetQuestVar(Net::MessageIn &msg)
{
    const int var = msg.readInt16("variable");
    const int val = msg.readInt32("value");
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

void QuestRecv::processPlayerQuests(Net::MessageIn &msg)
{
    const int count = (msg.readInt16("len") - 4) / 6;
    for (int f = 0; f < count; f ++)
    {
        const int var = msg.readInt16("variable");
        const int val = msg.readInt32("value");
        if (questsWindow)
            questsWindow->updateQuest(var, val);
        if (skillDialog)
            skillDialog->updateQuest(var, val);
    }
    if (questsWindow)
        questsWindow->rebuild(false);
}

}  // namespace TmwAthena
