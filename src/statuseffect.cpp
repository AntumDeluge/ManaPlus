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

#include "statuseffect.h"

#include "configuration.h"
#include "soundmanager.h"

#include "const/resources/spriteaction.h"

#include "gui/widgets/tabs/chat/chattab.h"

#include "particle/particle.h"

#include "listeners/debugmessagelistener.h"

#include "resources/beingcommon.h"

#include "resources/sprite/animatedsprite.h"

#include "debug.h"

StatusEffect::StatusEffect() :
    mMessage(),
    mSFXEffect(),
    mParticleEffect(),
    mIcon(),
    mAction(),
    mName(),
    mPersistentParticleEffect(false),
    mIsPoison(false),
    mIsCart(false),
    mIsRiding(false),
    mIsTrickDead(false),
    mIsPostDelay(false)
{
}

StatusEffect::~StatusEffect()
{
}

void StatusEffect::playSFX() const
{
    if (!mSFXEffect.empty())
        soundManager.playSfx(mSFXEffect);
}

void StatusEffect::deliverMessage() const
{
    if (!mMessage.empty() && localChatTab)
        localChatTab->chatLog(mMessage, ChatMsgType::BY_SERVER);
}

Particle *StatusEffect::getParticle() const
{
    if (!particleEngine || mParticleEffect.empty())
        return nullptr;
    else
        return particleEngine->addEffect(mParticleEffect, 0, 0);
}

AnimatedSprite *StatusEffect::getIcon() const
{
    if (mIcon.empty())
    {
        return nullptr;
    }
    else
    {
        return AnimatedSprite::load(paths.getStringValue(
            "sprites").append(mIcon));
    }
}

std::string StatusEffect::getAction() const
{
    if (mAction.empty())
        return SpriteAction::INVALID;
    else
        return mAction;
}
