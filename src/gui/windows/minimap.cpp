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

#include "gui/windows/minimap.h"

#include "actormanager.h"
#include "configuration.h"
#include "party.h"
#include "settings.h"

#include "being/localplayer.h"

#include "gui/popupmanager.h"
#include "gui/viewport.h"

#include "gui/popups/popupmenu.h"
#include "gui/popups/textpopup.h"

#include "gui/windows/setupwindow.h"

#include "resources/imagehelper.h"
#include "resources/resourcemanager.h"

#include "resources/map/map.h"
#include "resources/map/metatile.h"

#include "utils/gettext.h"
#include "utils/physfstools.h"
#include "utils/sdlcheckutils.h"

#include "debug.h"

Minimap *minimap = nullptr;
bool Minimap::mShow = true;

Minimap::Minimap() :
    // TRANSLATORS: mini map window name
    Window(_("Map"), false, nullptr, "map.xml"),
    mWidthProportion(0.5),
    mHeightProportion(0.5),
    mMapImage(nullptr),
    mMapOriginX(0),
    mMapOriginY(0),
    mCustomMapImage(false),
    mAutoResize(config.getBoolValue("autoresizeminimaps"))
{
    setWindowName("Minimap");
    mShow = config.getValueBool(getWindowName() + "Show", true);

    config.addListener("autoresizeminimaps", this);

    setDefaultSize(5, 25, 100, 100);
    // set this to false as the minimap window size is changed
    // depending on the map size
    setResizable(true);
    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    setDefaultVisible(true);
    setSaveVisible(true);

    setStickyButton(true);
    setSticky(false);

    loadWindowState();
    setVisible(mShow, isSticky());
    enableVisibleSound(true);
}

Minimap::~Minimap()
{
    config.setValue(getWindowName() + "Show", mShow);
    config.removeListeners(this);
    CHECKLISTENERS
    deleteMapImage();
}

void Minimap::deleteMapImage()
{
    if (mMapImage)
    {
        if (mCustomMapImage)
            delete mMapImage;
        else
            mMapImage->decRef();
        mMapImage = nullptr;
    }
}

void Minimap::setMap(const Map *const map)
{
    BLOCK_START("Minimap::setMap")
    std::string caption;

    if (map)
        caption = map->getName();

    if (caption.empty())
    {
        // TRANSLATORS: mini map window name
        caption = _("Map");
    }

    setCaption(caption);
    deleteMapImage();

    if (map)
    {
        if (config.getBoolValue("showExtMinimaps"))
        {
            SDL_Surface *const surface = MSDL_CreateRGBSurface(SDL_SWSURFACE,
                map->getWidth(), map->getHeight(), 32,
                0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
            if (!surface)
            {
                if (!isSticky())
                    setVisible(false);
                BLOCK_END("Minimap::setMap")
                return;
            }

            // I'm not sure if the locks are necessary since it's a SWSURFACE
            SDL_LockSurface(surface);
            int* data = static_cast<int*>(surface->pixels);
            if (!data)
            {
                if (!isSticky())
                    setVisible(false);
                BLOCK_END("Minimap::setMap")
                return;
            }
            const int size = surface->h * surface->w;
            const int mask = (BlockMask::WALL | BlockMask::AIR
                | BlockMask::WATER);

            for (int ptr = 0; ptr < size; ptr ++)
                *(data ++) = -!(map->mMetaTiles[ptr].blockmask & mask);

            SDL_UnlockSurface(surface);

            mMapImage = imageHelper->load(surface);
            mMapImage->setAlpha(settings.guiAlpha);
            mCustomMapImage = true;
            MSDL_FreeSurface(surface);
        }
        else
        {
            std::string tempname = paths.getStringValue("minimaps").append(
                map->getFilename()).append(".png");
            ResourceManager *const resman = ResourceManager::getInstance();

            std::string minimapName = map->getProperty("minimap");

            if (minimapName.empty() && PhysFs::exists(tempname.c_str()))
                minimapName = tempname;

            if (minimapName.empty())
            {
                tempname = std::string("graphics/minimaps/").append(
                    map->getFilename()).append(".png");
                if (PhysFs::exists(tempname.c_str()))
                    minimapName = tempname;
            }

            mMapImage = resman->getImage(minimapName);
            mCustomMapImage = false;
        }
    }

    if (mMapImage && map)
    {
        const int width = mMapImage->mBounds.w + 2 * getPadding();
        const int height = mMapImage->mBounds.h
            + getTitleBarHeight() + getPadding();
        const int mapWidth = mMapImage->mBounds.w < 100 ? width : 100;
        const int mapHeight = mMapImage->mBounds.h < 100 ? height : 100;
        const int minWidth = mapWidth > 310 ? 310 : mapWidth;
        const int minHeight = mapHeight > 220 ? 220 : mapHeight;

        setMinWidth(minWidth);
        setMinHeight(minHeight);

        mWidthProportion = static_cast<float>(
                mMapImage->mBounds.w) / static_cast<float>(map->getWidth());
        mHeightProportion = static_cast<float>(
                mMapImage->mBounds.h) / static_cast<float>(map->getHeight());

        setMaxWidth(width);
        setMaxHeight(height);
        if (mAutoResize)
        {
            setWidth(width);
            setHeight(height);
        }

        const Rect &rect = mDimension;
        setDefaultSize(rect.x, rect.y, rect.width, rect.height);
        resetToDefaultSize();

        if (mShow)
            setVisible(true);
    }
    else
    {
        if (!isSticky())
            setVisible(false);
    }
    BLOCK_END("Minimap::setMap")
}

void Minimap::toggle()
{
    setVisible(!isWindowVisible(), isSticky());
    mShow = isWindowVisible();
}

void Minimap::draw(Graphics *graphics)
{
    BLOCK_START("Minimap::draw")
    Window::draw(graphics);

    if (!userPalette || !localPlayer || !viewport)
    {
        BLOCK_END("Minimap::draw")
        return;
    }

    const Rect a = getChildrenArea();

    graphics->pushClipArea(a);

    if (!actorManager)
    {
        BLOCK_END("Minimap::draw")
        return;
    }

    mMapOriginX = 0;
    mMapOriginY = 0;

    if (mMapImage)
    {
        const SDL_Rect &rect = mMapImage->mBounds;
        const int w = rect.w;
        const int h = rect.h;
        if (w > a.width || h > a.height)
        {
            const Vector &p = localPlayer->getPosition();
            mMapOriginX = (a.width / 2) - (p.x + static_cast<float>(
                viewport->getCameraRelativeX()) * mWidthProportion) / 32;

            mMapOriginY = (a.height / 2) - (p.y + static_cast<float>(
                viewport->getCameraRelativeY()) * mHeightProportion) / 32;

            const int minOriginX = a.width - w;
            const int minOriginY = a.height - h;

            if (mMapOriginX < minOriginX)
                mMapOriginX = minOriginX;
            if (mMapOriginY < minOriginY)
                mMapOriginY = minOriginY;
            if (mMapOriginX > 0)
                mMapOriginX = 0;
            if (mMapOriginY > 0)
                mMapOriginY = 0;
        }

        graphics->drawImage(mMapImage, mMapOriginX, mMapOriginY);
    }

    const ActorSprites &actors = actorManager->getAll();
    FOR_EACH (ActorSpritesConstIterator, it, actors)
    {
        if (!(*it) || (*it)->getType() == ActorType::FloorItem)
            continue;

        const Being *const being = static_cast<const Being *const>(*it);
        if (!being)
            continue;

        int dotSize = 2;
        int type = UserPalette::PC;

        if (being == localPlayer)
        {
            type = UserPalette::SELF;
            dotSize = 3;
        }
        else if (being->isGM())
        {
            type = UserPalette::GM;
        }
        else if (being->getGuild() == localPlayer->getGuild()
                 || being->getGuildName() == localPlayer->getGuildName())
        {
            type = UserPalette::GUILD;
        }
        else
        {
            switch (being->getType())
            {
                case ActorType::Monster:
                    type = UserPalette::MONSTER;
                    break;

                case ActorType::Npc:
                    type = UserPalette::NPC;
                    break;

                case ActorType::Portal:
                    type = UserPalette::PORTAL_HIGHLIGHT;
                    break;

                case ActorType::LocalPet:
                case ActorType::Pet:
                    type = UserPalette::PET;
                    break;

                case ActorType::Mercenary:
                    type = UserPalette::MERCENARY;
                    break;

                case ActorType::Homunculus:
                    type = UserPalette::HOMUNCULUS;
                    break;

                case ActorType::Avatar:
                case ActorType::Unknown:
                case ActorType::Player:
                case ActorType::FloorItem:
                default:
                    continue;
            }
        }

        if (userPalette)
            graphics->setColor(userPalette->getColor(type));

        const int offsetHeight = static_cast<int>(static_cast<float>(
                dotSize - 1) * mHeightProportion);
        const int offsetWidth = static_cast<int>(static_cast<float>(
                dotSize - 1) * mWidthProportion);
        const Vector &pos = being->getPosition();

        graphics->fillRectangle(Rect(
            static_cast<float>(pos.x * mWidthProportion) / 32
            + mMapOriginX - offsetWidth,
            static_cast<float>(pos.y * mHeightProportion) / 32
            + mMapOriginY - offsetHeight, dotSize, dotSize));
    }

    if (localPlayer->isInParty())
    {
        const Party *const party = localPlayer->getParty();
        if (party)
        {
            const PartyMember *const m = party->getMember(
                localPlayer->getName());
            const Party::MemberList *const members = party->getMembers();
            if (m && members)
            {
                const std::string curMap = m->getMap();
                Party::MemberList::const_iterator it = members->begin();
                const Party::MemberList::const_iterator
                    it_end = members->end();
                while (it != it_end)
                {
                    const PartyMember *const member = *it;
                    if (member && member->getMap() == curMap
                        && member->getOnline() && member != m)
                    {
                        if (userPalette)
                        {
                            graphics->setColor(userPalette->getColor(
                                UserPalette::PARTY));
                        }

                        const int offsetHeight = static_cast<int>(
                            mHeightProportion);
                        const int offsetWidth = static_cast<int>(
                            mWidthProportion);

                        graphics->fillRectangle(Rect(
                            static_cast<int>(member->getX()
                            * mWidthProportion) + mMapOriginX - offsetWidth,
                            static_cast<int>(member->getY()
                            * mHeightProportion) + mMapOriginY - offsetHeight,
                            2, 2));
                    }
                    ++ it;
                }
            }
        }
    }

    const Vector &pos = localPlayer->getPosition();

    const int gw = graphics->getWidth();
    const int gh = graphics->getHeight();
    int x = static_cast<float>((pos.x - (gw / 2)
        + viewport->getCameraRelativeX())
        * mWidthProportion) / 32 + mMapOriginX;
    int y = static_cast<float>((pos.y - (gh / 2)
        + viewport->getCameraRelativeY())
        * mHeightProportion) / 32 + mMapOriginY;

    const int w = static_cast<int>(static_cast<float>(
        gw) * mWidthProportion / 32);
    const int h = static_cast<int>(static_cast<float>(
        gh) * mHeightProportion / 32);

    if (w <= a.width)
    {
        if (x < 0 && w)
            x = 0;
        if (x + w > a.width)
            x = a.width - w;
    }
    if (h <= a.height)
    {
        if (y < 0 && h)
            y = 0;
        if (y + h > a.height)
            y = a.height - h;
    }

    graphics->setColor(userPalette->getColor(UserPalette::PC));
    graphics->drawRectangle(Rect(x, y, w, h));
    graphics->popClipArea();
    BLOCK_END("Minimap::draw")
}

void Minimap::mousePressed(MouseEvent &event)
{
    if (event.getButton() == MouseButton::RIGHT)
        return;
    Window::mousePressed(event);
}

void Minimap::mouseReleased(MouseEvent &event)
{
    Window::mouseReleased(event);

    if (!localPlayer || !popupManager)
        return;

    if (event.getButton() == MouseButton::LEFT)
    {
        int x = event.getX();
        int y = event.getY();
        screenToMap(x, y);

        localPlayer->navigateTo(x, y);
    }
    else if (event.getButton() == MouseButton::RIGHT)
    {
        int x = event.getX();
        int y = event.getY();
        screenToMap(x, y);
        popupMenu->showMapPopup(viewport->mMouseX,
            viewport->mMouseY,
            x, y);
    }
}

void Minimap::mouseMoved(MouseEvent &event)
{
    Window::mouseMoved(event);
    const int x = event.getX();
    const int y = event.getY();
    const Rect &rect = mDimension;
    textPopup->show(x + rect.x, y + rect.y, mCaption);
}

void Minimap::mouseExited(MouseEvent &event)
{
    Window::mouseExited(event);
    textPopup->hide();
}

void Minimap::screenToMap(int &x, int &y)
{
    const Rect a = getChildrenArea();
    x = (x - a.x - mMapOriginX + mWidthProportion) / mWidthProportion;
    y = (y - a.y - mMapOriginY + mHeightProportion) / mHeightProportion;
}

void Minimap::optionChanged(const std::string &name)
{
    if (name == "autoresizeminimaps")
        mAutoResize = config.getBoolValue("autoresizeminimaps");
}
