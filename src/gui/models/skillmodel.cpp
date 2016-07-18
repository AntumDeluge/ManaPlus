/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/models/skillmodel.h"

#include "resources/skill/skilldata.h"

#include "debug.h"

SkillModel::SkillModel() :
    mSkills(),
    mVisibleSkills()
{
}

SkillInfo *SkillModel::getSkillAt(const int i) const
{
    if (i < 0 || i >= CAST_S32(mVisibleSkills.size()))
        return nullptr;
    return mVisibleSkills.at(i);
}

std::string SkillModel::getElementAt(int i)
{
    const SkillInfo *const info = getSkillAt(i);
    if (info)
        return info->data->name;
    else
        return std::string();
}

void SkillModel::updateVisibilities()
{
    mVisibleSkills.clear();

    FOR_EACH (SkillList::const_iterator, it, mSkills)
    {
        if ((*it) && (*it)->visible == Visible_true)
            mVisibleSkills.push_back((*it));
    }
}
