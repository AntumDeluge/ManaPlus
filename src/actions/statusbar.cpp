/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2015  The ManaPlus Developers
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

#include "actions/statusbar.h"

#include "game.h"
#include "soundmanager.h"

#include "actions/actiondef.h"

#include "being/localplayer.h"
#include "being/playerrelations.h"

#include "gui/viewport.h"

#include "gui/widgets/tabs/chat/chattab.h"

#include "listeners/updatestatuslistener.h"

#include "resources/map/map.h"

#include "utils/gettext.h"

#ifdef ANDROID
#ifndef USE_SDL2
#include <SDL_screenkeyboard.h>
#endif
#endif

#include "debug.h"

namespace Actions
{

impHandler0(switchQuickDrop)
{
    callYellowBarCond(changeQuickDropCounter)
}

impHandler0(changeCrazyMove)
{
    callYellowBar(changeCrazyMoveType);
}

impHandler0(changePickupType)
{
    callYellowBar(changePickUpType);
}

impHandler0(changeMoveType)
{
    callYellowBar(changeMoveType);
}

impHandler0(changeAttackWeaponType)
{
    callYellowBar(changeAttackWeaponType);
}

impHandler0(changeAttackType)
{
    callYellowBar(changeAttackType);
}

impHandler0(changeFollowMode)
{
    callYellowBar(changeFollowMode);
}

impHandler0(changeImitationMode)
{
    callYellowBar(changeImitationMode);
}

impHandler0(changeMagicAttackType)
{
    callYellowBar(changeMagicAttackType);
}

impHandler0(changePvpMode)
{
    callYellowBar(changePvpAttackType);
}

impHandler0(changeMoveToTarget)
{
    callYellowBar(changeMoveToTargetType);
}

impHandler0(changeGameModifier)
{
    if (localPlayer)
    {
        GameModifiers::changeGameModifiers(false);
        return true;
    }
    return false;
}

impHandler0(changeAudio)
{
    soundManager.changeAudio();
    if (localPlayer)
        localPlayer->updateMusic();
    return true;
}

impHandler0(away)
{
    GameModifiers::changeAwayMode(true);
    if (localPlayer)
    {
        localPlayer->updateStatus();
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    return false;
}

impHandler0(camera)
{
    if (viewport)
    {
        viewport->toggleCameraMode();
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    return false;
}

impHandler0(changeMapMode)
{
    if (viewport)
        viewport->toggleMapDrawType();
    UpdateStatusListener::distributeEvent();
    if (Game::instance())
    {
        if (Map *const map = Game::instance()->getCurrentMap())
            map->redrawMap();
    }
    return true;
}

impHandler0(changeTrade)
{
    unsigned int deflt = player_relations.getDefault();
    if (deflt & PlayerRelation::TRADE)
    {
        if (localChatTab)
        {
            // TRANSLATORS: disable trades message
            localChatTab->chatLog(_("Ignoring incoming trade requests"),
                ChatMsgType::BY_SERVER);
        }
        deflt &= ~PlayerRelation::TRADE;
    }
    else
    {
        if (localChatTab)
        {
            // TRANSLATORS: enable trades message
            localChatTab->chatLog(_("Accepting incoming trade requests"),
                ChatMsgType::BY_SERVER);
        }
        deflt |= PlayerRelation::TRADE;
    }

    player_relations.setDefault(deflt);
    return true;
}

}  // namespace Actions
