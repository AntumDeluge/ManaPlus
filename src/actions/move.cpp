/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2014  The ManaPlus Developers
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

#include "actions/move.h"

#include "game.h"

#include "actions/actiondef.h"

#include "being/localplayer.h"

#include "gui/windows/socialwindow.h"
#include "gui/windows/npcdialog.h"
#include "gui/windows/outfitwindow.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "debug.h"

namespace Actions
{

static bool closeMoveNpcDialog(bool focus)
{
    NpcDialog *const dialog = NpcDialog::getActive();
    if (dialog)
    {
        if (dialog->isCloseState())
        {
            dialog->closeDialog();
            return true;
        }
        else if (focus)
        {
            dialog->refocus();
        }
    }
    return false;
}

impHandler(moveUp)
{
    if (inputManager.isActionActive(InputAction::EMOTE))
        return directUp(event);
    return closeMoveNpcDialog(false);
}

impHandler(moveDown)
{
    if (inputManager.isActionActive(InputAction::EMOTE))
        return directDown(event);
    return closeMoveNpcDialog(false);
}

impHandler(moveLeft)
{
    if (outfitWindow && inputManager.isActionActive(InputAction::WEAR_OUTFIT))
    {
        outfitWindow->wearPreviousOutfit();
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    if (inputManager.isActionActive(InputAction::EMOTE))
        return directLeft(event);
    return closeMoveNpcDialog(false);
}

impHandler(moveRight)
{
    if (outfitWindow && inputManager.isActionActive(InputAction::WEAR_OUTFIT))
    {
        outfitWindow->wearNextOutfit();
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    if (inputManager.isActionActive(InputAction::EMOTE))
        return directRight(event);
    return closeMoveNpcDialog(false);
}

impHandler(moveForward)
{
    if (inputManager.isActionActive(InputAction::EMOTE))
        return directRight(event);
    return closeMoveNpcDialog(false);
}

impHandler(moveToPoint)
{
    const int num = event.action - InputAction::MOVE_TO_POINT_1;
    if (socialWindow && num >= 0)
    {
        socialWindow->selectPortal(num);
        return true;
    }

    return false;
}

impHandler0(crazyMoves)
{
    if (localPlayer)
    {
        localPlayer->crazyMove();
        return true;
    }
    return false;
}

impHandler0(moveToTarget)
{
    if (localPlayer && !inputManager.isActionActive(InputAction::TARGET_ATTACK)
        && !inputManager.isActionActive(InputAction::ATTACK))
    {
        localPlayer->moveToTarget();
        return true;
    }
    return false;
}

impHandler0(moveToHome)
{
    if (localPlayer && !inputManager.isActionActive(InputAction::TARGET_ATTACK)
        && !inputManager.isActionActive(InputAction::ATTACK))
    {
        localPlayer->moveToHome();
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    return false;
}

impHandler0(directUp)
{
    if (localPlayer)
    {
        if (localPlayer->getDirection() != BeingDirection::UP)
        {
//            if (PacketLimiter::limitPackets(PACKET_DIRECTION))
            {
                localPlayer->setDirection(BeingDirection::UP);
                if (playerHandler)
                    playerHandler->setDirection(BeingDirection::UP);
            }
        }
        return true;
    }
    return false;
}

impHandler0(directDown)
{
    if (localPlayer)
    {
        if (localPlayer->getDirection() != BeingDirection::DOWN)
        {
//            if (PacketLimiter::limitPackets(PACKET_DIRECTION))
            {
                localPlayer->setDirection(BeingDirection::DOWN);
                if (playerHandler)
                {
                    playerHandler->setDirection(
                        BeingDirection::DOWN);
                }
            }
        }
        return true;
    }
    return false;
}

impHandler0(directLeft)
{
    if (localPlayer)
    {
        if (localPlayer->getDirection() != BeingDirection::LEFT)
        {
//            if (PacketLimiter::limitPackets(PACKET_DIRECTION))
            {
                localPlayer->setDirection(BeingDirection::LEFT);
                if (playerHandler)
                {
                    playerHandler->setDirection(
                        BeingDirection::LEFT);
                }
            }
        }
        return true;
    }
    return false;
}

impHandler0(directRight)
{
    if (localPlayer)
    {
        if (localPlayer->getDirection() != BeingDirection::RIGHT)
        {
//            if (PacketLimiter::limitPackets(PACKET_DIRECTION))
            {
                localPlayer->setDirection(BeingDirection::RIGHT);
                if (playerHandler)
                {
                    playerHandler->setDirection(
                        BeingDirection::RIGHT);
                }
            }
        }
        return true;
    }
    return false;
}

}  // namespace Actions
