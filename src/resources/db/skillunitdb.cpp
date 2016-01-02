/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "resources/db/skillunitdb.h"

#include "configuration.h"
#include "logger.h"

#include "resources/beingcommon.h"
#include "resources/beinginfo.h"

#include "resources/sprite/spritereference.h"

#include "utils/gettext.h"
#include "utils/dtor.h"

#include "debug.h"

namespace
{
    BeingInfos mSkillUnitInfos;
    bool mLoaded = false;
}

void SkillUnitDb::load()
{
    if (mLoaded)
        unload();

    logger->log1("Initializing skill unit database...");
    loadXmlFile(paths.getStringValue("skillUnitsFile"));
    loadXmlFile(paths.getStringValue("skillUnitsPatchFile"));
    loadXmlDir("skillUnitsPatchDir", loadXmlFile);
    mLoaded = true;
}

void SkillUnitDb::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName, UseResman_true, SkipError_false);
    const XmlNodePtrConst rootNode = doc.rootNode();

    if (!rootNode || !xmlNameEqual(rootNode, "skillunits"))
    {
        logger->log("SkillUnitDb: Error while loading %s!",
            fileName.c_str());
        return;
    }

    // iterate <skillunit>s
    for_each_xml_child_node(skillUnitNode, rootNode)
    {
        if (xmlNameEqual(skillUnitNode, "include"))
        {
            const std::string name = XML::getProperty(skillUnitNode,
                "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }
        else if (!xmlNameEqual(skillUnitNode, "skillunit"))
        {
            continue;
        }

        const BeingTypeId id = fromInt(XML::getProperty(
            skillUnitNode, "id", -1), BeingTypeId);
        if (id == BeingTypeId_negOne)
        {
            logger->log("SkillUnitDb: skill unit with missing ID in %s!",
                paths.getStringValue("skillUnitsFile").c_str());
            continue;
        }

        BeingInfo *currentInfo = nullptr;
        if (mSkillUnitInfos.find(id) != mSkillUnitInfos.end())
            currentInfo = mSkillUnitInfos[id];
        if (!currentInfo)
            currentInfo = new BeingInfo;

        currentInfo->setName(XML::langProperty(skillUnitNode,
            // TRANSLATORS: unknown info name
            "name", _("skill")));

        currentInfo->setTargetSelection(XML::getBoolProperty(skillUnitNode,
            "targetSelection", false));

        BeingCommon::readBasicAttributes(currentInfo, skillUnitNode, "talk");

        currentInfo->setDeadSortOffsetY(XML::getProperty(skillUnitNode,
            "deadSortOffsetY", 31));

        SpriteDisplay display;
        for_each_xml_child_node(spriteNode, skillUnitNode)
        {
            if (!spriteNode->xmlChildrenNode)
                continue;

            if (xmlNameEqual(spriteNode, "sprite"))
            {
                SpriteReference *const currentSprite = new SpriteReference;
                currentSprite->sprite = reinterpret_cast<const char*>(
                    spriteNode->xmlChildrenNode->content);
                currentSprite->variant =
                    XML::getProperty(spriteNode, "variant", 0);
                display.sprites.push_back(currentSprite);
            }
            else if (xmlNameEqual(spriteNode, "particlefx"))
            {
                std::string particlefx = reinterpret_cast<const char*>(
                    spriteNode->xmlChildrenNode->content);
                display.particles.push_back(particlefx);
            }
        }

        currentInfo->setDisplay(display);

        mSkillUnitInfos[id] = currentInfo;
    }
}

void SkillUnitDb::unload()
{
    delete_all(mSkillUnitInfos);
    mSkillUnitInfos.clear();

    mLoaded = false;
}

BeingInfo *SkillUnitDb::get(const BeingTypeId id)
{
    const BeingInfoIterator i = mSkillUnitInfos.find(id);

    if (i == mSkillUnitInfos.end())
    {
        logger->log("SkillUnitDb: Warning, unknown skill unit id %d requested",
            toInt(id, int));
        return BeingInfo::unknown;
    }
    else
    {
        return i->second;
    }
}
