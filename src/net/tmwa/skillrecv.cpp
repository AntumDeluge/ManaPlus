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

#include "net/tmwa/skillrecv.h"

#include "notifymanager.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/skilldialog.h"

#include "net/messagein.h"

#include "net/ea/skillrecv.h"

#include "net/tmwa/messageout.h"
#include "net/tmwa/protocol.h"

#include "utils/gettext.h"

#include "debug.h"

namespace TmwAthena
{

void SkillRecv::processPlayerSkills(Net::MessageIn &msg)
{
    msg.readInt16("len");
    const int skillCount = (msg.getLength() - 4) / 37;
    int updateSkill = 0;

    for (int k = 0; k < skillCount; k++)
    {
        const int skillId = msg.readInt16("skill id");
        const SkillType::SkillType inf = static_cast<SkillType::SkillType>(
            msg.readInt16("inf"));
        msg.readInt16("skill pool flags");
        const int level = msg.readInt16("skill level");
        const int sp = msg.readInt16("sp");
        const int range = msg.readInt16("range");
        msg.skip(24, "unused");
        const Modifiable up = fromBool(msg.readUInt8("up flag"), Modifiable);
        const int oldLevel = PlayerInfo::getSkillLevel(skillId);
        if (oldLevel && oldLevel != level)
            updateSkill = skillId;
        PlayerInfo::setSkillLevel(skillId, level);
        if (skillDialog)
        {
            if (!skillDialog->updateSkill(skillId, range, up, inf, sp))
            {
                skillDialog->addSkill(SkillOwner::Player,
                    skillId, "", level, range, up, inf, sp);
            }
        }
    }
    if (skillDialog)
    {
        skillDialog->update();
        if (updateSkill)
            skillDialog->playUpdateEffect(updateSkill);
    }
}

void SkillRecv::processSkillFailed(Net::MessageIn &msg)
{
    // Action failed (ex. sit because you have not reached the
    // right level)
    const int skillId   = msg.readInt16("skill id");
    const int16_t bskill  = msg.readInt16("bskill");
    msg.readInt16("btype");
    const signed char success = msg.readUInt8("success");
    const signed char reason  = msg.readUInt8("reason");
    if (success != static_cast<int>(SKILL_FAILED)
        && bskill == static_cast<int>(BSKILL_EMOTE))
    {
        logger->log("Action: %d/%d", bskill, success);
    }

    std::string txt;
    if (success == static_cast<int>(SKILL_FAILED)
        && skillId == static_cast<int>(SKILL_BASIC))
    {
        if (localPlayer &&
            bskill == static_cast<int>(BSKILL_EMOTE) &&
            reason == static_cast<int>(RFAIL_SKILLDEP))
        {
            localPlayer->stopAdvert();
        }

        switch (bskill)
        {
            case BSKILL_TRADE:
                // TRANSLATORS: error message
                txt = _("Trade failed!");
                break;
            case BSKILL_EMOTE:
                // TRANSLATORS: error message
                txt = _("Emote failed!");
                break;
            case BSKILL_SIT:
                // TRANSLATORS: error message
                txt = _("Sit failed!");
                break;
            case BSKILL_CREATECHAT:
                // TRANSLATORS: error message
                txt = _("Chat creating failed!");
                break;
            case BSKILL_JOINPARTY:
                // TRANSLATORS: error message
                txt = _("Could not join party!");
                break;
            case BSKILL_SHOUT:
                // TRANSLATORS: error message
                txt = _("Cannot shout!");
                break;
            default:
                UNIMPLIMENTEDPACKET;
                break;
        }

        txt.append(" ");

        switch (reason)
        {
            case RFAIL_SKILLDEP:
                // TRANSLATORS: error message
                txt.append(_("You have not yet reached a high enough lvl!"));
                break;
            case RFAIL_INSUFHP:
                // TRANSLATORS: error message
                txt.append(_("Insufficient HP!"));
                break;
            case RFAIL_INSUFSP:
                // TRANSLATORS: error message
                txt.append(_("Insufficient SP!"));
                break;
            case RFAIL_NOMEMO:
                // TRANSLATORS: error message
                txt.append(_("You have no memos!"));
                break;
            case RFAIL_SKILLDELAY:
                // TRANSLATORS: error message
                txt.append(_("You cannot do that right now!"));
                break;
            case RFAIL_ZENY:
                // TRANSLATORS: error message
                txt.append(_("Seems you need more money... ;-)"));
                break;
            case RFAIL_WEAPON:
                // TRANSLATORS: error message
                txt.append(_("You cannot use this skill with that "
                    "kind of weapon!"));
                break;
            case RFAIL_REDGEM:
                // TRANSLATORS: error message
                txt.append(_("You need another red gem!"));
                break;
            case RFAIL_BLUEGEM:
                // TRANSLATORS: error message
                txt.append(_("You need another blue gem!"));
                break;
            case RFAIL_OVERWEIGHT:
                // TRANSLATORS: error message
                txt.append(_("You're carrying to much to do this!"));
                break;
            default:
                // TRANSLATORS: error message
                txt.append(_("Huh? What's that?"));
                UNIMPLIMENTEDPACKET;
                break;
        }
    }
    else
    {
        switch (skillId)
        {
            case SKILL_WARP :
                // TRANSLATORS: error message
                txt = _("Warp failed...");
                break;
            case SKILL_STEAL :
                // TRANSLATORS: error message
                txt = _("Could not steal anything...");
                break;
            case SKILL_ENVENOM :
                // TRANSLATORS: error message
                txt = _("Poison had no effect...");
                break;
            default:
                UNIMPLIMENTEDPACKET;
                break;
        }
    }

    NotifyManager::notify(NotifyTypes::SKILL_FAIL_MESSAGE, txt);
}

}  // namespace TmwAthena
