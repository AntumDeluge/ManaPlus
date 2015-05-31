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

#include "gui/windows/ministatuswindow.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "inventory.h"

#include "enums/being/attributes.h"

#include "enums/gui/progresscolorid.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "gui/popupmanager.h"
#include "gui/skin.h"

#include "gui/popups/popupmenu.h"
#include "gui/popups/statuspopup.h"
#include "gui/popups/textpopup.h"

#include "gui/windows/statuswindow.h"

#include "gui/widgets/progressbar.h"

#include "net/playerhandler.h"
#include "net/gamehandler.h"

#include "utils/delete2.h"
#include "utils/dtor.h"
#include "utils/gettext.h"

#include "debug.h"

MiniStatusWindow *miniStatusWindow = nullptr;
extern volatile int tick_time;

typedef std::vector <ProgressBar*>::const_iterator ProgressBarVectorCIter;

MiniStatusWindow::MiniStatusWindow() :
    Window("MiniStatus", Modal_false, nullptr, "ministatus.xml"),
    InventoryListener(),
    AttributeListener(),
    StatListener(),
    ArrowsListener(),
    UpdateStatusListener(),
    mBars(),
    mBarNames(),
    mIcons(),
    // TRANSLATORS: status bar name
    mHpBar(createBar(0, 100, 0, ThemeColorId::HP_BAR, ProgressColorId::PROG_HP,
        "hpprogressbar.xml", "hpprogressbar_fill.xml",
        "hp bar", _("health bar"))),
    mMpBar(gameHandler->canUseMagicBar()
        ? createBar(0, 100, 0, playerHandler->canUseMagic()
        ? ThemeColorId::MP_BAR : ThemeColorId::NO_MP_BAR,
        playerHandler->canUseMagic()
        ? ProgressColorId::PROG_MP : ProgressColorId::PROG_NO_MP,
        playerHandler->canUseMagic()
        ? "mpprogressbar.xml" : "nompprogressbar.xml",
        playerHandler->canUseMagic()
        ? "mpprogressbar_fill.xml" : "nompprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "mp bar", _("mana bar")) : nullptr),
    mXpBar(createBar(0, 100, 0, ThemeColorId::XP_BAR, ProgressColorId::PROG_EXP,
        "xpprogressbar.xml", "xpprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "xp bar", _("experience bar"))),
    mJobBar(nullptr),
    mWeightBar(createBar(0, 140, 0, ThemeColorId::WEIGHT_BAR, ProgressColorId::PROG_WEIGHT,
        "weightprogressbar.xml", "weightprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "weight bar", _("weight bar"))),
    mInvSlotsBar(createBar(0, 45, 0,
        ThemeColorId::SLOTS_BAR, ProgressColorId::PROG_INVY_SLOTS,
        "slotsprogressbar.xml", "slotsprogressbar_fill.xml",
        "inventory slots bar",
        // TRANSLATORS: status bar name
        _("inventory slots bar"))),
    mMoneyBar(createBar(0, 130, 0, ThemeColorId::MONEY_BAR, ProgressColorId::PROG_MONEY,
        "moneyprogressbar.xml", "moneyprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "money bar", _("money bar"))),
    mArrowsBar(createBar(0, 50, 0, ThemeColorId::ARROWS_BAR, ProgressColorId::PROG_ARROWS,
        "arrowsprogressbar.xml", "arrowsprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "arrows bar", _("arrows bar"))),
    mStatusBar(createBar(100, (config.getIntValue("fontSize") > 16
        ? 250 : 165), 0, ThemeColorId::STATUS_BAR, ProgressColorId::PROG_STATUS,
        "statusprogressbar.xml", "statusprogressbar_fill.xml",
        // TRANSLATORS: status bar name
        "status bar", _("status bar"))),
    mStatusPopup(new StatusPopup),
    mSpacing(mSkin ? mSkin->getOption("spacing", 3) : 3),
    mIconPadding(mSkin ? mSkin->getOption("iconPadding", 3) : 3),
    mIconSpacing(mSkin ? mSkin->getOption("iconSpacing", 2) : 2),
    mMaxX(0)
{
    mStatusPopup->postInit();

    StatusWindow::updateHPBar(mHpBar);

    if (gameHandler->canUseMagicBar() && statusWindow)
        statusWindow->updateMPBar(mMpBar);

    const bool job = serverConfig.getValueBool("showJob", true);

    StatusWindow::updateXPBar(mXpBar);

    if (job)
    {
        mJobBar = createBar(0, 100, 0, ThemeColorId::JOB_BAR, ProgressColorId::PROG_JOB,
            // TRANSLATORS: status bar name
            "jobprogressbar.xml", "jobprogressbar_fill.xml",
            "job bar", _("job bar"));
        StatusWindow::updateJobBar(mJobBar);
    }

    loadBars();
    updateBars();

    setVisible(Visible_true);
    addMouseListener(this);
    Inventory *const inv = PlayerInfo::getInventory();
    if (inv)
        inv->addInventoyListener(this);

    StatusWindow::updateMoneyBar(mMoneyBar);
    StatusWindow::updateArrowsBar(mArrowsBar);
    updateStatus();
}

MiniStatusWindow::~MiniStatusWindow()
{
    delete2(mStatusPopup);
    delete_all(mIcons);
    mIcons.clear();

    Inventory *const inv = PlayerInfo::getInventory();
    if (inv)
        inv->removeInventoyListener(this);

    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        ProgressBar *bar = *it;
        if (!bar)
            continue;
        if (!bar->isVisibleLocal())
            delete bar;
    }
    mBars.clear();
}

ProgressBar *MiniStatusWindow::createBar(const float progress,
                                         const int width,
                                         const int height,
                                         const ThemeColorId textColor,
                                         const ProgressColorIdT backColor,
                                         const std::string &restrict skin,
                                         const std::string &restrict skinFill,
                                         const std::string &restrict name,
                                         const std::string &restrict
                                         description)
{
    ProgressBar *const bar = new ProgressBar(this,
        progress, width, height, backColor, skin, skinFill);
    bar->setActionEventId(name);
    bar->setId(description);
    bar->setColor(getThemeColor(textColor), getThemeColor(textColor + 1));
    mBars.push_back(bar);
    mBarNames[name] = bar;
    return bar;
}

void MiniStatusWindow::updateBars()
{
    int x = 0;
    const ProgressBar *lastBar = nullptr;
    FOR_EACH (ProgressBarVectorCIter, it, mBars)
        remove(*it);

    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        ProgressBar *const bar = *it;
        if (!bar)
            continue;
        if (bar->isVisibleLocal())
        {
            bar->setPosition(x, 0);
            add(bar);
            x += bar->getWidth() + mSpacing;
            lastBar = bar;
        }
    }

    if (lastBar)
    {
        setContentSize(lastBar->getX() + lastBar->getWidth(),
            lastBar->getY() + lastBar->getHeight());
    }
    mMaxX = x;
}

void MiniStatusWindow::setIcon(const int index, AnimatedSprite *const sprite)
{
    if (index >= static_cast<int>(mIcons.size()))
        mIcons.resize(index + 1, nullptr);

    delete mIcons[index];
    mIcons[index] = sprite;
}

void MiniStatusWindow::eraseIcon(const int index)
{
    if (index < static_cast<int>(mIcons.size()))
    {
        delete mIcons[index];
        mIcons.erase(mIcons.begin() + index);
    }
}

void MiniStatusWindow::drawIcons(Graphics *const graphics)
{
    // Draw icons
    int icon_x = mMaxX + mIconPadding;
    for (size_t i = 0, sz = mIcons.size(); i < sz; i ++)
    {
        const AnimatedSprite *const icon = mIcons[i];
        if (icon)
        {
            icon->draw(graphics, icon_x, mIconPadding);
            icon_x += mIconSpacing + icon->getWidth();
        }
    }
}

void MiniStatusWindow::statChanged(const AttributesT id A_UNUSED,
                                   const int oldVal1 A_UNUSED,
                                   const int oldVal2 A_UNUSED)
{
    if (statusWindow)
        statusWindow->updateMPBar(mMpBar);
    StatusWindow::updateJobBar(mJobBar);
}

void MiniStatusWindow::attributeChanged(const AttributesT id,
                                        const int oldVal A_UNUSED,
                                        const int newVal A_UNUSED)
{
    switch (id)
    {
        case Attributes::HP:
        case Attributes::MAX_HP:
            StatusWindow::updateHPBar(mHpBar);
            break;
        case Attributes::MP:
        case Attributes::MAX_MP:
            statusWindow->updateMPBar(mMpBar);
            break;
        case Attributes::EXP:
        case Attributes::EXP_NEEDED:
            StatusWindow::updateXPBar(mXpBar);
            break;
        case Attributes::TOTAL_WEIGHT:
        case Attributes::MAX_WEIGHT:
            StatusWindow::updateWeightBar(mWeightBar);
            break;
        case Attributes::MONEY:
            StatusWindow::updateMoneyBar(mMoneyBar);
            break;
        default:
            break;
    }
}

void MiniStatusWindow::updateStatus()
{
    statusWindow->updateStatusBar(mStatusBar);
    if (mStatusPopup && mStatusPopup->isPopupVisible())
        mStatusPopup->update();
}

void MiniStatusWindow::logic()
{
    BLOCK_START("MiniStatusWindow::logic")
    Window::logic();

    for (size_t i = 0, sz = mIcons.size(); i < sz; i++)
    {
        AnimatedSprite *const icon = mIcons[i];
        if (icon)
            icon->update(tick_time * 10);
    }
    BLOCK_END("MiniStatusWindow::logic")
}

void MiniStatusWindow::draw(Graphics *graphics)
{
    BLOCK_START("MiniStatusWindow::draw")
    drawChildren(graphics);
    BLOCK_END("MiniStatusWindow::draw")
}

void MiniStatusWindow::safeDraw(Graphics *graphics)
{
    BLOCK_START("MiniStatusWindow::draw")
    safeDrawChildren(graphics);
    BLOCK_END("MiniStatusWindow::draw")
}

void MiniStatusWindow::mouseMoved(MouseEvent &event)
{
    Window::mouseMoved(event);

    const int x = event.getX();
    const int y = event.getY();

    const Rect &rect = mDimension;
    if (event.getSource() == mStatusBar)
    {
        mStatusPopup->view(x + rect.x, y + rect.y);
        textPopup->hide();
    }
    else if (event.getSource() == mXpBar)
    {
        std::string level;
        if (localPlayer && localPlayer->isGM())
        {
            // TRANSLATORS: status bar label
            level = strprintf(_("Level: %d (GM %d)"),
                PlayerInfo::getAttribute(Attributes::LEVEL),
                localPlayer->getGMLevel());
        }
        else
        {
            // TRANSLATORS: status bar label
            level = strprintf(_("Level: %d"),
                PlayerInfo::getAttribute(Attributes::LEVEL));
        }

        const int exp = PlayerInfo::getAttribute(Attributes::EXP);
        const int expNeed = PlayerInfo::getAttribute(Attributes::EXP_NEEDED);
        if (exp > expNeed)
        {
            textPopup->show(x + rect.x, y + rect.y, level, strprintf("%d/%d",
                exp, expNeed));
        }
        else
        {
            textPopup->show(x + rect.x, y + rect.y, level, strprintf("%d/%d",
                exp, expNeed),
                // TRANSLATORS: status bar label
                strprintf("%s: %d", _("Need"), expNeed - exp));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mHpBar)
    {
        textPopup->show(x + rect.x, y + rect.y, event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(Attributes::HP),
            PlayerInfo::getAttribute(Attributes::MAX_HP)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mMpBar)
    {
        textPopup->show(x + rect.x, y + rect.y, event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(Attributes::MP),
            PlayerInfo::getAttribute(Attributes::MAX_MP)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mJobBar)
    {
        const std::pair<int, int> exp =  PlayerInfo::getStatExperience(
            Attributes::JOB);

        // TRANSLATORS: job bar label
        const std::string level = strprintf(_("Job level: %d"),
            PlayerInfo::getStatBase(Attributes::JOB));

        if (exp.first > exp.second)
        {
            textPopup->show(x + rect.x, y + rect.y, level,
                strprintf("%d/%d", exp.first, exp.second));
        }
        else
        {
            textPopup->show(x + rect.x, y + rect.y, level,
                strprintf("%d/%d", exp.first, exp.second),
                // TRANSLATORS: status bar label
                strprintf("%s: %d", _("Need"), exp.second - exp.first));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mWeightBar)
    {
        textPopup->show(x + rect.x, y + rect.y, event.getSource()->getId(),
            strprintf("%d/%d", PlayerInfo::getAttribute(
            Attributes::TOTAL_WEIGHT),
            PlayerInfo::getAttribute(Attributes::MAX_WEIGHT)));
        mStatusPopup->hide();
    }
    else if (event.getSource() == mInvSlotsBar)
    {
        const Inventory *const inv = PlayerInfo::getInventory();
        if (inv)
        {
            const int usedSlots = inv->getNumberOfSlotsUsed();
            const int maxSlots = inv->getSize();
            textPopup->show(x + rect.x, y + rect.y,
                event.getSource()->getId(),
                strprintf("%d/%d", usedSlots, maxSlots));
        }
        mStatusPopup->hide();
    }
    else if (event.getSource() == mMoneyBar)
    {
        textPopup->show(x + rect.x, y + rect.y,
            event.getSource()->getId(),
            toString(PlayerInfo::getAttribute(Attributes::MONEY)));
    }
    else
    {
        textPopup->hide();
        mStatusPopup->hide();
    }
}

void MiniStatusWindow::mousePressed(MouseEvent &event)
{
    if (event.getButton() == MouseButton::RIGHT)
    {
        const ProgressBar *const bar = dynamic_cast<ProgressBar*>(
            event.getSource());
        if (!bar)
            return;
        event.consume();
        if (popupManager)
        {
            popupMenu->showPopup(getX() + event.getX(),
                getY() + event.getY(), bar);
        }
    }
}

void MiniStatusWindow::mouseExited(MouseEvent &event)
{
    Window::mouseExited(event);

    textPopup->hide();
    mStatusPopup->hide();
}

void MiniStatusWindow::showBar(const std::string &name,
                               const Visible visible)
{
    ProgressBar *const bar = mBarNames[name];
    if (!bar)
        return;
    bar->setVisible(visible);
    updateBars();
    saveBars();
}

void MiniStatusWindow::loadBars()
{
    if (!config.getIntValue("ministatussaved"))
    {
        if (mWeightBar)
            mWeightBar->setVisible(Visible_false);
        if (mInvSlotsBar)
            mInvSlotsBar->setVisible(Visible_false);
        if (mMoneyBar)
            mMoneyBar->setVisible(Visible_false);
        if (mArrowsBar)
            mArrowsBar->setVisible(Visible_false);
        if (mStatusBar)
            mStatusBar->setVisible(Visible_false);
        if (mJobBar)
            mJobBar->setVisible(Visible_true);
        return;
    }

    for (int f = 0; f < 10; f ++)
    {
        const std::string str = config.getValue(
            "ministatus" + toString(f), "");
        if (str == "")
            continue;
        ProgressBar *const bar = mBarNames[str];
        if (!bar)
            continue;
        bar->setVisible(Visible_false);
    }
}

void MiniStatusWindow::saveBars() const
{
    int i = 0;
    FOR_EACH (ProgressBarVectorCIter, it, mBars)
    {
        const ProgressBar *const bar = *it;
        if (!bar->isVisibleLocal())
        {
            config.setValue("ministatus" + toString(i),
                bar->getActionEventId());
            i ++;
        }
    }
    for (int f = i; f < 10; f ++)
        config.deleteKey("ministatus" + toString(f));

    config.setValue("ministatussaved", true);
}

void MiniStatusWindow::slotsChanged(Inventory *const inventory)
{
    if (!inventory)
        return;

    if (inventory->getType() == InventoryType::INVENTORY)
        StatusWindow::updateInvSlotsBar(mInvSlotsBar);
}

Rect MiniStatusWindow::getChildrenArea()
{
    const int padding = mPadding;
    const int padding2 = padding * 2;
    const Rect &rect = mDimension;
    return Rect(padding, padding,
        rect.width - padding2,
        rect.height - padding2);
}

void MiniStatusWindow::arrowsChanged()
{
    StatusWindow::updateArrowsBar(mArrowsBar);
}

#ifdef USE_PROFILER
void MiniStatusWindow::logicChildren()
{
    BLOCK_START("MiniStatusWindow::logicChildren")
    BasicContainer::logicChildren();
    BLOCK_END("MiniStatusWindow::logicChildren")
}
#endif
