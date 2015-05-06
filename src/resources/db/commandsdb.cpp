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

#include "resources/db/commandsdb.h"

#include "configuration.h"
#include "logger.h"
#include "textcommand.h"

#include "resources/beingcommon.h"

#include "debug.h"

namespace
{
    CommandsMap mCommands;
    bool mLoaded = false;
}

void CommandsDB::load()
{
    if (mLoaded)
        unload();
    loadXmlFile(paths.getStringValue("defaultCommandsFile"));
    loadXmlFile(paths.getStringValue("defaultCommandsPatchFile"));
    loadXmlDir("defaultCommandsPatchDir", loadXmlFile);
    mLoaded = true;
}

static SpellTarget parseTarget(const std::string &text)
{
    if (text == "allow target")
        return ALLOWTARGET;
    else if (text == "need target")
        return NEEDTARGET;
    else
        return NOTARGET;
}

void CommandsDB::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName, UseResman_true, SkipError_false);
    const XmlNodePtrConst rootNode = doc.rootNode();

    if (!rootNode || !xmlNameEqual(rootNode, "commands"))
    {
        logger->log("Commands Database: Error while loading %s!",
            fileName.c_str());
        return;
    }

    for_each_xml_child_node(commandNode, rootNode)
    {
        if (xmlNameEqual(commandNode, "include"))
        {
            const std::string name = XML::getProperty(commandNode, "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }

        if (!xmlNameEqual(commandNode, "command"))
            continue;

        const int id = XML::getProperty(commandNode, "id", -1);
        if (id == -1)
            continue;

        CommandsMapIter it = mCommands.find(id);
        if (it != mCommands.end())
        {
            logger->log("Commands database: duplicate id: %d", id);
            TextCommand *tempCmd = (*it).second;
            mCommands.erase(it);
            delete tempCmd;
        }
        const std::string name = XML::langProperty(
            commandNode, "name", "?");
        const std::string command = XML::getProperty(
            commandNode, "command", "");
        const std::string comment = XML::getProperty(
            commandNode, "comment", "");
        const SpellTarget targetType = parseTarget(XML::getProperty(
            commandNode, "target", ""));
        const std::string icon = XML::getProperty(
            commandNode, "icon", "");
        const int skill1 = XML::getIntProperty(
            commandNode, "skill1", 0, 0, 1000000);
        const int level1 = XML::getIntProperty(
            commandNode, "level1", 0, 0, 1000);
        const int skill2 = XML::getIntProperty(
            commandNode, "skill2", 0, 0, 1000000);
        const int level2 = XML::getIntProperty(
            commandNode, "level2", 0, 0, 1000);
        const int mana = XML::getIntProperty(
            commandNode, "mana", 0, 0, 100000);

        TextCommand *cmd = nullptr;
        if (skill1)
        {
            cmd = new TextCommand(id, name, command, comment,
                targetType, icon, level1, static_cast<MagicSchool>(skill2),
                level2, mana);
        }
        else
        {
            cmd = new TextCommand(id, name, command, comment,
                targetType, icon);
        }
        mCommands[id] = cmd;
    }
}

void CommandsDB::unload()
{
    mCommands.clear();
    mLoaded = false;
}

std::map<int, TextCommand*> &CommandsDB::getAll()
{
    return mCommands;
}
