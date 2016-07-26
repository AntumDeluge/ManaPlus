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

#include "gui/widgets/statspagebasic.h"

#include "being/playerinfo.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/vertcontainer.h"

#include "gui/widgets/attrs/changedisplay.h"

#include "gui/windows/chatwindow.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "resources/db/statdb.h"

#include "debug.h"

StatsPageBasic::StatsPageBasic(const Widget2 *const widget) :
    Container(widget),
    WidgetListener(),
    StatListener(),
    mAttrs(),
    mAttrCont(new VertContainer(this, 32)),
    mAttrScroll(new ScrollArea(this, mAttrCont, false)),
    mCharacterPointsLabel(new Label(this, "C"))
{
    addWidgetListener(this);

    mAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);

    add(mAttrScroll);
    const std::vector<BasicStat> &basicStats = StatDb::getBasicStats();
    FOR_EACH (std::vector<BasicStat>::const_iterator, it, basicStats)
    {
        const BasicStat &stat = *it;
        AttrDisplay *disp = new ChangeDisplay(this, stat.attr, stat.name, stat.tag);
        disp->update();
        mAttrCont->add2(disp, true);
        mAttrs[stat.attr] = disp;
    }

    // TRANSLATORS: status window label
    mCharacterPointsLabel->setCaption(strprintf(_("Character points: %d"),
        PlayerInfo::getAttribute(Attributes::PLAYER_CHAR_POINTS)));
    mCharacterPointsLabel->adjustSize();
    mAttrCont->add1(mCharacterPointsLabel);
}

void StatsPageBasic::widgetResized(const Event &event A_UNUSED)
{
    mAttrScroll->setSize(getWidth(), getHeight());
}

void StatsPageBasic::statChanged(const AttributesT id,
                                 const int oldVal1 A_UNUSED,
                                 const int oldVal2 A_UNUSED)
{
    const Attrs::const_iterator it = mAttrs.find(id);
    if (it != mAttrs.end() && it->second)
        it->second->update();
}

void StatsPageBasic::attributeChanged(const AttributesT id,
                                      const int oldVal A_UNUSED,
                                      const int newVal)
{
    PRAGMA45(GCC diagnostic push)
    PRAGMA45(GCC diagnostic ignored "-Wswitch-enum")
    switch (id)
    {
        case Attributes::PLAYER_CHAR_POINTS:
            mCharacterPointsLabel->setCaption(strprintf(
                // TRANSLATORS: status window label
                _("Character points: %d"), newVal));

            mCharacterPointsLabel->adjustSize();
            // Update all attributes
            for (Attrs::const_iterator it = mAttrs.begin();
                 it != mAttrs.end(); ++it)
            {
                if (it->second)
                    it->second->update();
            }
            return;

        case Attributes::PLAYER_CORR_POINTS:
            // Update all attributes
            for (Attrs::const_iterator it = mAttrs.begin();
                 it != mAttrs.end(); ++it)
            {
                if (it->second)
                    it->second->update();
            }
            return;

        default:
            break;
    }
    PRAGMA45(GCC diagnostic pop)
    const Attrs::const_iterator it = mAttrs.find(id);
    if (it != mAttrs.end() && it->second)
        it->second->update();
}

void StatsPageBasic::setPointsNeeded(const AttributesT id,
                                     const int needed)
{
    const Attrs::const_iterator it = mAttrs.find(id);

    if (it != mAttrs.end())
    {
        AttrDisplay *const disp = it->second;
        if (disp && disp->getType() == AttrDisplay::CHANGEABLE)
            static_cast<ChangeDisplay*>(disp)->setPointsNeeded(needed);
    }
}

std::string StatsPageBasic::getStatsStr() const
{
    Attrs::const_iterator it = mAttrs.begin();
    const Attrs::const_iterator it_end = mAttrs.end();
    std::string str;
    while (it != it_end)
    {
        const ChangeDisplay *const attr = dynamic_cast<ChangeDisplay*>(
            (*it).second);
        if (attr)
        {
            str.append(strprintf("%s:%s ", attr->getShortName().c_str(),
                attr->getValue().c_str()));
        }
        ++ it;
    }
    return str;
}
