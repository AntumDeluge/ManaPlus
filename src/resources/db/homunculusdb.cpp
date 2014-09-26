/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#include "resources/db/homunculusdb.h"

#include "logger.h"

#include "resources/beingcommon.h"
#include "resources/beinginfo.h"

#include "resources/map/blockmask.h"

#include "utils/dtor.h"

#include "configuration.h"

#include "debug.h"

namespace
{
    BeingInfos mHomunculusInfos;
    bool mLoaded = false;
}

void HomunculusDB::load()
{
    if (mLoaded)
        unload();

    logger->log1("Initializing homunculus database...");
    loadXmlFile(paths.getStringValue("homunculusesFile"));
    loadXmlFile(paths.getStringValue("homunculusesPatchFile"));
    loadXmlDir("homunculusesPatchDir", loadXmlFile);

    mLoaded = true;
}

void HomunculusDB::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName);
    const XmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlNameEqual(rootNode, "homunculuses"))
    {
        logger->log("Homunculus Database: Error while loading %s!",
            paths.getStringValue("homunculusesFile").c_str());
        mLoaded = true;
        return;
    }

    const int offset = XML::getProperty(rootNode, "offset", 0);

    // iterate <homunculus>s
    for_each_xml_child_node(homunculusNode, rootNode)
    {
        if (xmlNameEqual(homunculusNode, "include"))
        {
            const std::string name = XML::getProperty(
                homunculusNode, "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }
        if (!xmlNameEqual(homunculusNode, "homunculus"))
            continue;

        const int id = XML::getProperty(homunculusNode, "id", 0);
        BeingInfo *currentInfo = nullptr;
        if (mHomunculusInfos.find(id + offset) != mHomunculusInfos.end())
        {
            logger->log("HomunculusDB: Redefinition of homunculus ID %d", id);
            currentInfo = mHomunculusInfos[id + offset];
        }
        if (!currentInfo)
            currentInfo = new BeingInfo;

        currentInfo->setBlockType(BlockType::MONSTER);
        BeingCommon::readBasicAttributes(currentInfo,
            homunculusNode, "attack");

        currentInfo->setMaxHP(XML::getProperty(homunculusNode, "maxHP", 0));

        currentInfo->setDeadSortOffsetY(XML::getProperty(
            homunculusNode, "deadSortOffsetY", 31));

        currentInfo->setColorsList(XML::getProperty(homunculusNode,
            "colors", ""));

        unsigned char block = 0;
        std::string walkStr = XML::getProperty(
            homunculusNode, "walkType", "walk");
        if (walkStr == "walk")
            block = BlockMask::WATER | BlockMask::AIR;
        else if (walkStr == "fly")
            block = 0;
        else if (walkStr == "swim")
            block = BlockMask::GROUND | BlockMask::AIR;
        else if (walkStr == "walkswim" || walkStr == "swimwalk")
            block = BlockMask::AIR;

        currentInfo->setBlockWalkMask(static_cast<unsigned char>(
            BlockMask::WALL | block));

        if (currentInfo->getMaxHP())
            currentInfo->setStaticMaxHP(true);

        SpriteDisplay display;

        // iterate <sprite>s and <sound>s
        for_each_xml_child_node(spriteNode, homunculusNode)
        {
            BeingCommon::readObjectNodes(spriteNode, display,
                currentInfo, "HomunculusDB");
        }
        currentInfo->setDisplay(display);

        mHomunculusInfos[id + offset] = currentInfo;
    }
}

void HomunculusDB::unload()
{
    delete_all(mHomunculusInfos);
    mHomunculusInfos.clear();

    mLoaded = false;
}


BeingInfo *HomunculusDB::get(const int id)
{
    BeingInfoIterator i = mHomunculusInfos.find(id);

    if (i == mHomunculusInfos.end())
    {
        i = mHomunculusInfos.find(id);
        if (i == mHomunculusInfos.end())
        {
            logger->log("HomunculusDB: Warning, unknown homunculus ID "
                "%d requested",
                id);
            return BeingInfo::unknown;
        }
        else
        {
            return i->second;
        }
    }
    else
    {
        return i->second;
    }
}
