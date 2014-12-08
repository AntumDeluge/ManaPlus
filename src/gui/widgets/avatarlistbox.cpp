/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "gui/widgets/avatarlistbox.h"

#include "actormanager.h"
#include "configuration.h"
#include "graphicsvertexes.h"

#include "being/localplayer.h"

#include "gui/gui.h"
#include "gui/popupmanager.h"
#include "gui/skin.h"
#include "gui/viewport.h"

#include "gui/fonts/font.h"

#include "gui/popups/popupmenu.h"

#include "gui/models/avatarlistmodel.h"

#include "gui/windows/chatwindow.h"

#include "resources/image.h"
#include "resources/mapitemtype.h"

#include "resources/map/map.h"

#include "debug.h"

int AvatarListBox::instances = 0;
Image *AvatarListBox::onlineIcon = nullptr;
Image *AvatarListBox::offlineIcon = nullptr;

AvatarListBox::AvatarListBox(const Widget2 *const widget,
                             AvatarListModel *const model) :
    ListBox(widget, model, "avatarlistbox.xml"),
    mImagePadding(mSkin ? mSkin->getOption("imagePadding") : 0),
    mShowGender(config.getBoolValue("showgender")),
    mShowLevel(config.getBoolValue("showlevel"))
{
    instances++;

    if (instances == 1)
    {
        onlineIcon = Theme::getImageFromThemeXml("circle-on.xml", "");
        offlineIcon = Theme::getImageFromThemeXml("circle-off.xml", "");
    }

    setWidth(200);

    config.addListener("showgender", this);
    config.addListener("showlevel", this);

    mForegroundColor = getThemeColor(Theme::TEXT);
    mForegroundColor2 = getThemeColor(Theme::TEXT_OUTLINE);
}

AvatarListBox::~AvatarListBox()
{
    config.removeListeners(this);
    CHECKLISTENERS

    instances--;

    if (instances == 0)
    {
        if (onlineIcon)
        {
            onlineIcon->decRef();
            onlineIcon = nullptr;
        }
        if (offlineIcon)
        {
            offlineIcon->decRef();
            offlineIcon = nullptr;
        }
    }
}

void AvatarListBox::draw(Graphics *graphics)
{
    BLOCK_START("AvatarListBox::draw")
    if (!mListModel || !localPlayer)
    {
        BLOCK_END("AvatarListBox::draw")
        return;
    }

    const Widget *const parent = mParent;
    if (!parent)
        return;

    AvatarListModel *const model = static_cast<AvatarListModel *const>(
        mListModel);
    updateAlpha();

    Font *const font = getFont();
    const int fontHeight = getFont()->getHeight();
    const std::string &name = localPlayer->getName();

    // Draw the list elements
    graphics->setColorAll(mForegroundColor, mForegroundColor2);
    ImageCollection vertexes;
    const bool useCaching = isBatchDrawRenders(openGLMode);

    for (int i = 0, y = 0;
         i < model->getNumberOfElements();
         ++i, y += fontHeight)
    {
        const Avatar *const a = model->getAvatarAt(i);
        if (!a)
            continue;

        const MapItemType::Type type = static_cast<MapItemType::Type>(
            a->getType());
        if (type != MapItemType::SEPARATOR)
        {
            // Draw online status
            const Image *const icon = a->getOnline()
                ? onlineIcon : offlineIcon;
            if (icon)
            {
                if (useCaching)
                {
                    graphics->calcTileCollection(&vertexes, icon,
                        mImagePadding, y + mPadding);
                }
                else
                {
                    graphics->drawImage(icon, mImagePadding, y + mPadding);
                }
            }
        }
    }

    if (useCaching)
    {
        graphics->finalize(&vertexes);
        graphics->drawTileCollection(&vertexes);
    }

    graphics->setColorAll(mForegroundColor, mForegroundColor2);

    for (int i = 0, y = 0;
         i < model->getNumberOfElements();
         ++i, y += fontHeight)
    {
        const Avatar *const a = model->getAvatarAt(i);
        if (!a)
            continue;

        const MapItemType::Type type = static_cast<MapItemType::Type>(
            a->getType());
        std::string text;

        if (a->getMaxHp() > 0)
        {
            if (mShowLevel && a->getLevel() > 1)
            {
                text = strprintf("%s %d/%d (%d)", a->getComplexName().c_str(),
                                 a->getHp(), a->getMaxHp(), a->getLevel());
            }
            else
            {
                text = strprintf("%s %d/%d", a->getComplexName().c_str(),
                                 a->getHp(), a->getMaxHp());
            }
            const bool isPoison = a->getPoison();
            if (a->getMaxHp())
            {
                const int themeColor = (isPoison
                    ? Theme::PROG_HP_POISON : Theme::PROG_HP);
                Color color = Theme::getProgressColor(
                    themeColor, static_cast<float>(a->getHp())
                    / static_cast<float>(a->getMaxHp()));
                color.a = 80;
                graphics->setColor(color);
                graphics->fillRectangle(Rect(mPadding, y + mPadding,
                    parent->getWidth() * a->getHp() / a->getMaxHp()
                    - 2 * mPadding, fontHeight));
            }
        }
        else if (a->getDamageHp() != 0 && a->getName() != name)
        {
            if (mShowLevel && a->getLevel() > 1)
            {
                text = strprintf("%s -%d (%d)", a->getComplexName().c_str(),
                                 a->getDamageHp(), a->getLevel());
            }
            else
            {
                text = strprintf("%s -%d", a->getComplexName().c_str(),
                                 a->getDamageHp());
            }

            const int themeColor = (a->getPoison()
                ? Theme::PROG_HP_POISON : Theme::PROG_HP);
            Color color = Theme::getProgressColor(themeColor, 1);
            color.a = 80;
            graphics->setColor(color);
            graphics->fillRectangle(Rect(mPadding, y + mPadding,
                parent->getWidth() * a->getDamageHp() / 1024
                - 2 * mPadding, fontHeight));

            if (a->getLevel() > 1)
            {
                graphics->setColor(mForegroundColor);
                int minHp = 40 + ((a->getLevel() - 1) * 5);
                if (minHp < 0)
                    minHp = 40;

                graphics->drawLine(parent->getWidth()*minHp / 1024
                    + mPadding, y + mPadding,
                    parent->getWidth() * minHp / 1024, y + fontHeight);
            }
        }
        else
        {
            if (mShowLevel && a->getLevel() > 1)
            {
                text = strprintf("%s (%d)", a->getComplexName().c_str(),
                                 a->getLevel());
            }
            else
            {
                text = a->getComplexName();
            }
        }

        if (!a->getMap().empty())
        {
            if (a->getX() != -1)
            {
                text.append(strprintf(" [%d,%d %s]", a->getX(), a->getY(),
                    a->getMap().c_str()));
            }
            else
            {
                text.append(strprintf(" [%s]", a->getMap().c_str()));
            }
        }

        if (graphics->getSecure())
        {
            if (mShowGender)
            {
                switch (a->getGender())
                {
                    case Gender::FEMALE:
                        text.append(" \u2640 ");
                        break;
                    case Gender::MALE:
                        text.append(" \u2642 ");
                        break;
                    default:
                    case Gender::UNSPECIFIED:
                    case Gender::OTHER:
                        break;
                }
            }
        }
        else
        {
            if (mShowGender)
            {
                switch (a->getGender())
                {
                    case Gender::FEMALE:
                        text.append(strprintf(" \u2640 %s",
                            a->getAdditionString().c_str()));
                        break;
                    case Gender::MALE:
                        text.append(strprintf(" \u2642 %s",
                            a->getAdditionString().c_str()));
                        break;
                    default:
                    case Gender::UNSPECIFIED:
                    case Gender::OTHER:
                        break;
                }
            }
            else
            {
                text.append(a->getAdditionString());
            }
        }

        graphics->setColor(mForegroundColor);

        // Draw Name
        if (a->getDisplayBold())
        {
            if (type == MapItemType::SEPARATOR)
            {
                boldFont->drawString(graphics, text,
                    mImagePadding + mPadding, y + mPadding);
            }
            else
            {
                boldFont->drawString(graphics, text,
                    15 + mImagePadding + mPadding, y + mPadding);
            }
        }
        else
        {
            if (type == MapItemType::SEPARATOR)
            {
                font->drawString(graphics, text, mImagePadding + mPadding,
                    y + mPadding);
            }
            else
            {
                font->drawString(graphics, text, 15 + mImagePadding + mPadding,
                    y + mPadding);
            }
        }
    }

    setWidth(parent->getWidth() - 10);
    BLOCK_END("AvatarListBox::draw")
}

void AvatarListBox::mousePressed(MouseEvent &event)
{
    if (!actorManager || !localPlayer || !popupManager
        || !getFont()->getHeight())
    {
        return;
    }

    const int y = (event.getY() - mPadding) / getFont()->getHeight();
    if (!mListModel || y > mListModel->getNumberOfElements())
        return;

    setSelected(y);
    distributeActionEvent();
    const int selected = getSelected();
    AvatarListModel *const model = static_cast<AvatarListModel *const>(
        mListModel);
    if (!model)
        return;
    const Avatar *ava = model->getAvatarAt(selected);
    if (!ava)
        return;

    const MapItemType::Type type = static_cast<MapItemType::Type>(
        ava->getType());

    event.consume();
    const unsigned int eventButton = event.getButton();
    if (eventButton == MouseButton::LEFT)
    {
        if (type == MapItemType::EMPTY)
        {
            const Being *const being = actorManager->findBeingByName(
                ava->getName(), ActorType::Player);
            if (being)
                actorManager->heal(being);
        }
        else
        {
            localPlayer->navigateTo(ava->getX(), ava->getY());
        }
    }
    else if (eventButton == MouseButton::RIGHT)
    {
        switch (type)
        {
            case MapItemType::EMPTY:
            {
                const Avatar *const avatar = model->getAvatarAt(selected);
                if (avatar)
                {
                    const Being *const being = actorManager->findBeingByName(
                        avatar->getName(), ActorType::Player);
                    if (being)
                    {
                        popupMenu->showPopup(viewport->mMouseX,
                            viewport->mMouseY,
                            being);
                    }
                    else
                    {
                        popupMenu->showPlayerPopup(avatar->getName());
                    }
                }
                break;
            }
            case MapItemType::ATTACK:
            case MapItemType::PRIORITY:
            case MapItemType::IGNORE_:
            {
                std::string name;
                if (model->getAvatarAt(selected)->getLevel() == 0)
                    name.clear();
                else
                    name = model->getAvatarAt(selected)->getName();

                popupMenu->showAttackMonsterPopup(viewport->mMouseX,
                    viewport->mMouseY,
                    name,
                    static_cast<ActorType::Type>(model->getAvatarAt(
                    selected)->getType()));
                break;
            }
            case MapItemType::PICKUP:
            case MapItemType::NOPICKUP:
            {
                std::string name;
                if (model->getAvatarAt(selected)->getLevel() == 0)
                    name.clear();
                else
                    name = model->getAvatarAt(selected)->getName();

                popupMenu->showPickupItemPopup(viewport->mMouseX,
                    viewport->mMouseY,
                    name);
                break;
            }
            case MapItemType::HOME:
            case MapItemType::ROAD:
            case MapItemType::CROSS:
            case MapItemType::ARROW_UP:
            case MapItemType::ARROW_DOWN:
            case MapItemType::ARROW_LEFT:
            case MapItemType::ARROW_RIGHT:
            case MapItemType::PORTAL:
            case MapItemType::MUSIC:
            case MapItemType::SEPARATOR:
            {
                break;
            }
            default:
            {
                const Map *const map = viewport->getMap();
                ava = model->getAvatarAt(selected);
                if (map && ava)
                {
                    MapItem *const mapItem = map->findPortalXY(
                        ava->getX(), ava->getY());
                    popupMenu->showPopup(viewport->mMouseX,
                        viewport->mMouseY,
                        mapItem);
                }
                break;
            }
        }
    }
    else if (eventButton == MouseButton::MIDDLE)
    {
        if (type == MapItemType::EMPTY && chatWindow)
        {
            const std::string &name = model->getAvatarAt(selected)->getName();
            const WhisperTab *const tab = chatWindow->addWhisperTab(
                name, name, true);
            if (tab)
                chatWindow->saveState();
        }
    }
}

void AvatarListBox::mouseReleased(MouseEvent &event A_UNUSED)
{
}

void AvatarListBox::optionChanged(const std::string &value)
{
    if (value == "showgender")
        mShowGender = config.getBoolValue("showgender");
    else if (value == "showlevel")
        mShowLevel = config.getBoolValue("showlevel");
}
