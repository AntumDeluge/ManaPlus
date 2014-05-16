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

#include "gui/windows/skilldialog.h"

#include "configuration.h"
#include "dragdrop.h"
#include "effectmanager.h"
#include "itemshortcut.h"

#include "being/attributes.h"
#include "being/localplayer.h"

#include "gui/font.h"
#include "gui/skin.h"
#include "gui/viewport.h"

#include "gui/popups/textpopup.h"

#include "gui/models/skillmodel.h"

#include "gui/windows/setupwindow.h"
#include "gui/windows/shortcutwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/skilllistbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tabbedarea.h"

#include "gui/widgets/tabs/skilltab.h"
#include "gui/widgets/tabs/tab.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/skillhandler.h"

#include "utils/delete2.h"
#include "utils/dtor.h"
#include "utils/gettext.h"

#include "resources/beingcommon.h"

#include "debug.h"

SkillDialog::SkillDialog() :
    // TRANSLATORS: skills dialog name
    Window(_("Skills"), false, nullptr, "skills.xml"),
    ActionListener(),
    mSkills(),
    mTabs(new TabbedArea(this)),
    mDeleteTabs(),
    mPointsLabel(new Label(this, "0")),
    // TRANSLATORS: skills dialog button
    mUseButton(new Button(this, _("Use"), "use", this)),
    // TRANSLATORS: skills dialog button
    mIncreaseButton(new Button(this, _("Up"), "inc", this)),
    mDefaultModel(nullptr)
{
    mTabs->postInit();
    setWindowName("Skills");
    setCloseButton(true);
    setResizable(true);
    setSaveVisible(true);
    setStickyButtonLock(true);
    setDefaultSize(windowContainer->getWidth() - 280, 30, 275, 425);
    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    mUseButton->setEnabled(false);
    mIncreaseButton->setEnabled(false);

    place(0, 0, mTabs, 5, 5);
    place(0, 5, mPointsLabel, 4);
    place(3, 5, mUseButton);
    place(4, 5, mIncreaseButton);
}

void SkillDialog::postInit()
{
    setLocationRelativeTo(getParent());
    loadWindowState();
    enableVisibleSound(true);
}

SkillDialog::~SkillDialog()
{
    clearSkills();
}

void SkillDialog::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();
    if (eventId == "inc")
    {
        const SkillTab *const tab = static_cast<const SkillTab *const>(
            mTabs->getSelectedTab());
        if (tab)
        {
            if (const SkillInfo *const info = tab->getSelectedInfo())
            {
                Net::getPlayerHandler()->increaseSkill(
                    static_cast<uint16_t>(info->id));
            }
        }
    }
    else if (eventId == "sel")
    {
        const SkillTab *const tab = static_cast<const SkillTab *const>(
            mTabs->getSelectedTab());
        if (tab)
        {
            if (const SkillInfo *const info = tab->getSelectedInfo())
            {
                mUseButton->setEnabled(info->range > 0);
                mIncreaseButton->setEnabled(info->id < SKILL_VAR_MIN_ID);
                const int num = itemShortcutWindow->getTabIndex();
                if (num >= 0 && num < static_cast<int>(SHORTCUT_TABS)
                    && itemShortcut[num])
                {
                    itemShortcut[num]->setItemSelected(
                        info->id + SKILL_MIN_ID);
                }
            }
            else
            {
                mUseButton->setEnabled(false);
                mIncreaseButton->setEnabled(false);
            }
        }
    }
    else if (eventId == "use")
    {
        const SkillTab *const tab = static_cast<const SkillTab *const>(
            mTabs->getSelectedTab());
        if (tab)
        {
            const SkillInfo *const info = tab->getSelectedInfo();
            if (info && player_node && player_node->getTarget())
            {
                const Being *const being = player_node->getTarget();
                if (being)
                {
                    Net::getSkillHandler()->useBeing(info->level,
                        info->id, being->getId());
                }
            }
        }
    }
    else if (eventId == "close")
    {
        setVisible(false);
    }
}

std::string SkillDialog::update(const int id)
{
    const SkillMap::const_iterator i = mSkills.find(id);

    if (i != mSkills.end())
    {
        SkillInfo *const info = i->second;
        if (info)
        {
            info->update();
            return info->data->name;
        }
    }

    return std::string();
}

void SkillDialog::update()
{
    // TRANSLATORS: skills dialog label
    mPointsLabel->setCaption(strprintf(_("Skill points available: %d"),
        PlayerInfo::getAttribute(Attributes::SKILL_POINTS)));
    mPointsLabel->adjustSize();

    FOR_EACH (SkillMap::const_iterator, it, mSkills)
    {
        if ((*it).second && (*it).second->modifiable)
            (*it).second->update();
    }
}

void SkillDialog::clearSkills()
{
    mTabs->removeAll();
    mDeleteTabs.clear();
    mDefaultModel = nullptr;

    delete_all(mSkills);
    mSkills.clear();
}

void SkillDialog::loadSkills()
{
    clearSkills();
    loadXmlFile(paths.getStringValue("skillsFile"));
    if (mSkills.empty())
        loadXmlFile(paths.getStringValue("skillsFile2"));
    loadXmlFile(paths.getStringValue("skillsPatchFile"));
    loadXmlDir("skillsPatchDir", loadXmlFile);

    update();
}

void SkillDialog::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName);
    XmlNodePtrConst root = doc.rootNode();

    int setCount = 0;

    if (!root || !xmlNameEqual(root, "skills"))
    {
        logger->log("Error loading skills: " + fileName);
        return;
    }

    for_each_xml_child_node(set, root)
    {
        if (xmlNameEqual(set, "include"))
        {
            const std::string name = XML::getProperty(set, "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }
        else if (xmlNameEqual(set, "set"))
        {
            setCount++;
            const std::string setName = XML::getProperty(set, "name",
                // TRANSLATORS: skills dialog default skill tab
                strprintf(_("Skill Set %d"), setCount));

            SkillModel *const model = new SkillModel();
            if (!mDefaultModel)
                mDefaultModel = model;

            for_each_xml_child_node(node, set)
            {
                if (xmlNameEqual(node, "skill"))
                {
                    int id = XML::getIntProperty(node, "id", -1, -1, 1000000);
                    if (id == -1)
                    {
                        id = XML::getIntProperty(node, "var", -1, -1, 100000);
                        if (id == -1)
                            continue;
                        id += SKILL_VAR_MIN_ID;
                    }

                    SkillInfo *skill = getSkill(id);
                    if (!skill)
                    {
                        skill = new SkillInfo;
                        skill->id = static_cast<unsigned int>(id);
                        skill->modifiable = false;
                        skill->visible = false;
                        skill->model = model;
                        skill->update();
                        model->addSkill(skill);
                        mSkills[id] = skill;
                    }

                    std::string name = XML::langProperty(node, "name",
                        // TRANSLATORS: skills dialog. skill id
                        strprintf(_("Skill %d"), id));
                    std::string icon = XML::getProperty(node, "icon", "");
                    const int level = XML::getProperty(node, "level", 0);
                    SkillData *data = skill->getData(level);
                    if (!data)
                        data = new SkillData();

                    data->name = name;
                    data->setIcon(icon);
                    if (skill->id < SKILL_VAR_MIN_ID)
                    {
                        data->dispName = strprintf("%s, %u",
                            name.c_str(), skill->id);
                    }
                    else
                    {
                        data->dispName = strprintf("%s, (%u)",
                            name.c_str(), skill->id - SKILL_VAR_MIN_ID);
                    }
                    data->shortName = XML::langProperty(node,
                        "shortName", name.substr(0, 3));
                    data->description = XML::langProperty(
                        node, "description", "");
                    data->particle = XML::getProperty(
                        node, "particle", "");

                    data->soundHit.sound = XML::getProperty(
                        node, "soundHit", "");
                    data->soundHit.delay = XML::getProperty(
                        node, "soundHitDelay", 0);
                    data->soundMiss.sound = XML::getProperty(
                        node, "soundMiss", "");
                    data->soundMiss.delay = XML::getProperty(
                        node, "soundMissDelay", 0);

                    skill->addData(level, data);
                }
            }

            model->updateVisibilities();

            // possible leak listbox, scroll
            SkillListBox *const listbox = new SkillListBox(this, model);
            listbox->setActionEventId("sel");
            listbox->addActionListener(this);
            ScrollArea *const scroll = new ScrollArea(this, listbox, false);
            scroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
            scroll->setVerticalScrollPolicy(ScrollArea::SHOW_ALWAYS);

            SkillTab *const tab = new SkillTab(this, setName, listbox);
            mDeleteTabs.push_back(tab);

            mTabs->addTab(tab, scroll);
        }
    }
}

bool SkillDialog::updateSkill(const int id, const int range,
                              const bool modifiable)
{
    const SkillMap::const_iterator it = mSkills.find(id);

    if (it != mSkills.end())
    {
        SkillInfo *const info = it->second;
        if (info)
        {
            info->modifiable = modifiable;
            info->range = range;
            info->update();
        }
        return true;
    }
    return false;
}

void SkillDialog::addSkill(const int id, const int level, const int range,
                           const bool modifiable)
{
    if (mDefaultModel)
    {
        SkillInfo *const skill = new SkillInfo;
        skill->id = static_cast<unsigned int>(id);
        SkillData *const data = skill->data;
        data->name = "Unknown skill Id: " + toString(id);
        data->dispName = data->name;
        data->description.clear();
        data->setIcon("");
        skill->modifiable = modifiable;
        skill->visible = false;
        skill->model = mDefaultModel;
        skill->level = level;
        // TRANSLATORS: skills dialog. skill level
        skill->skillLevel = strprintf(_("Lvl: %d"), level);
        skill->range = range;
        skill->update();

        mDefaultModel->addSkill(skill);

        mSkills[id] = skill;
        mDefaultModel->updateVisibilities();
    }
}

SkillInfo* SkillDialog::getSkill(const int id) const
{
    SkillMap::const_iterator it = mSkills.find(id);
    if (it != mSkills.end())
        return (*it).second;
    return nullptr;
}

SkillInfo* SkillDialog::getSkillByItem(const int itemId) const
{
    SkillMap::const_iterator it = mSkills.find(itemId - SKILL_MIN_ID);
    if (it != mSkills.end())
        return (*it).second;
    return nullptr;
}

void SkillDialog::widgetResized(const Event &event)
{
    Window::widgetResized(event);

    if (mTabs)
        mTabs->adjustSize();
}

void SkillDialog::useItem(const int itemId) const
{
    const std::map<int, SkillInfo*>::const_iterator
        it = mSkills.find(itemId - SKILL_MIN_ID);
    if (it == mSkills.end())
        return;

    const SkillInfo *const info = (*it).second;
    if (info && player_node && player_node->getTarget())
    {
        const Being *const being = player_node->getTarget();
        if (being)
        {
            Net::getSkillHandler()->useBeing(info->level,
                info->id, being->getId());
        }
    }
}

void SkillDialog::updateTabSelection()
{
    const SkillTab *const tab = static_cast<SkillTab*>(
        mTabs->getSelectedTab());
    if (tab)
    {
        if (const SkillInfo *const info = tab->getSelectedInfo())
        {
            mUseButton->setEnabled(info->range > 0);
            mIncreaseButton->setEnabled(info->id < SKILL_VAR_MIN_ID);
        }
        else
        {
            mUseButton->setEnabled(false);
        }
    }
}

void SkillDialog::updateQuest(const int var, const int val)
{
    const int id = var + SKILL_VAR_MIN_ID;
    const SkillMap::const_iterator it = mSkills.find(id);

    if (it != mSkills.end())
    {
        SkillInfo *const info = it->second;
        if (info)
        {
            PlayerInfo::setSkillLevel(id, val);
            info->level = val;
            info->update();
        }
    }
}

void SkillDialog::playUpdateEffect(const int id) const
{
    const int effectId = paths.getIntValue("skillLevelUpEffectId");
    if (!effectManager || effectId == -1)
        return;
    const SkillMap::const_iterator it = mSkills.find(id);
    if (it != mSkills.end())
    {
        if (it->second)
            effectManager->trigger(effectId, player_node);
    }
}
