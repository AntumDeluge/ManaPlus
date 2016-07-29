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

#include "net/eathena/beingrecv.h"

#include "actormanager.h"
#include "effectmanager.h"
#include "game.h"
#include "notifymanager.h"
#include "party.h"

#include "being/mercenaryinfo.h"

#include "const/utils/timer.h"

#include "enums/resources/notifytypes.h"

#include "particle/particleengine.h"

#include "input/keyboardconfig.h"

#include "gui/viewport.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/socialwindow.h"
#include "gui/windows/outfitwindow.h"

#include "net/character.h"
#include "net/charserverhandler.h"
#include "net/messagein.h"
#include "net/serverfeatures.h"

#include "net/ea/beingrecv.h"

#include "net/eathena/maptypeproperty2.h"
#include "net/eathena/sp.h"
#include "net/eathena/sprite.h"

#include "resources/iteminfo.h"
#include "resources/db/itemdb.h"

#include "resources/map/map.h"

#include "utils/timer.h"

#include "debug.h"

extern int serverVersion;

namespace EAthena
{

void BeingRecv::processBeingChangeLook2(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));
    const uint8_t type = msg.readUInt8("type");

    const int id = msg.readInt16("id1");
    unsigned int id2 = msg.readInt16("id2");
    if (type != 2)
        id2 = 1;

    if (!localPlayer || !dstBeing)
        return;

    processBeingChangeLookContinue(msg, dstBeing, type, id, id2, nullptr);
}

void BeingRecv::processBeingChangeLookCards(Net::MessageIn &msg)
{
    Being *dstBeing = nullptr;
    int cards[maxCards];

    if (!actorManager)
    {   // here can be look from char server
        if (!serverFeatures->haveAdvancedSprites())
            return;
        Net::Characters &chars = Net::CharServerHandler::mCharacters;
        const BeingId id = msg.readBeingId("being id");

        FOR_EACH (Net::Characters::iterator, it, chars)
        {
            const Net::Character *const character = *it;
            if (character->dummy && character->dummy->getId() == id)
            {
                dstBeing = character->dummy;
                break;
            }
        }
    }
    else
    {
        dstBeing = actorManager->findBeing(
            msg.readBeingId("being id"));
    }

    const uint8_t type = msg.readUInt8("type");

    const int id = msg.readInt16("id1");
    unsigned int id2 = msg.readInt16("id2");
    if (type != 2)
        id2 = 1;

    for (int f = 0; f < maxCards; f ++)
        cards[f] = msg.readUInt16("card");

    if (!dstBeing)
        return;

    processBeingChangeLookContinue(msg, dstBeing, type, id, id2, &cards[0]);
}

void BeingRecv::processBeingChangeLookContinue(const Net::MessageIn &msg,
                                               Being *const dstBeing,
                                               const uint8_t type,
                                               const int id,
                                               const int id2,
                                               const int *const cards)
{
    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setOtherTime();

    switch (type)
    {
        // here should be used SPRITE_* constants
        // but for now they conflicting with sprites
        // SPRITE_* is same with server LOOK_*
        case 0:  // change race
            dstBeing->setSubtype(fromInt(id, BeingTypeId),
                dstBeing->getLook());
            break;
        case 1:  // eAthena LOOK_HAIR
            dstBeing->setHairColor(fromInt(id, ItemColor));
            dstBeing->setHairColorSpriteID(SPRITE_HAIR_COLOR,
                id * -1);
            break;
        case 2:  // LOOK_WEAPON Weapon ID in id, Shield ID in id2
            dstBeing->setSpriteCards(SPRITE_BODY,
                id,
                CardsList(cards));
            dstBeing->setWeaponId(id);
            dstBeing->setSpriteId(SPRITE_FLOOR,
                id2);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_FLOOR);
            break;
        case 3:  // LOOK_HEAD_BOTTOM
            dstBeing->setSpriteCards(SPRITE_WEAPON,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_WEAPON);
            break;
        case 4:  // LOOK_HEAD_TOP Change upper headgear for eAthena, hat for us
            dstBeing->setSpriteCards(SPRITE_CLOTHES_COLOR,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_CLOTHES_COLOR);
            break;
        case 5:  // LOOK_HEAD_MID Change middle headgear for eathena,
                 // armor for us
            dstBeing->setSpriteCards(SPRITE_HEAD_BOTTOM,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_BOTTOM);
            break;
        case 6:  // eAthena LOOK_HAIR_COLOR
            dstBeing->setHairColor(fromInt(id, ItemColor));
            dstBeing->setSpriteColor(SPRITE_HAIR_COLOR,
                ItemDB::get(dstBeing->getSpriteID(
                SPRITE_HAIR_COLOR)).getDyeColorsString(
                fromInt(id, ItemColor)));
            break;
        case 7:  // Clothes color. Now used as look
            dstBeing->setLook(CAST_U8(id));
            break;
        case 8:  // eAthena LOOK_SHIELD
            dstBeing->setSpriteCards(SPRITE_FLOOR,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_FLOOR);
            break;
        case 9:  // eAthena LOOK_SHOES
            dstBeing->setSpriteCards(SPRITE_HAIR,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HAIR);
            break;
        case 10:  // LOOK_GLOVES
            dstBeing->setSpriteCards(SPRITE_SHOES,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_SHOES);
            break;
        case 11:  // LOOK_FLOOR
            dstBeing->setSpriteCards(SPRITE_SHIELD,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_SHIELD);
            break;
        case 12:  // LOOK_ROBE
            dstBeing->setSpriteCards(SPRITE_HEAD_TOP,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_TOP);
            break;
        case 13:  // COSTUME_HEAD_TOP
            dstBeing->setSpriteCards(SPRITE_HEAD_MID,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_MID);
            break;
        case 14:  // COSTUME_HEAD_MID
            dstBeing->setSpriteCards(SPRITE_ROBE,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_ROBE);
            break;
        case 15:  // COSTUME_HEAD_LOW
            dstBeing->setSpriteCards(SPRITE_EVOL2,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL2);
            break;
        case 16:  // COSTUME_GARMENT
            dstBeing->setSpriteCards(SPRITE_EVOL3,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL3);
            break;
        case 17:  // ARMOR
            dstBeing->setSpriteCards(SPRITE_EVOL4,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL4);
            break;
        case 18:
            dstBeing->setSpriteCards(SPRITE_EVOL5,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL5);
            break;
        case 19:
            dstBeing->setSpriteCards(SPRITE_EVOL6,
                id,
                CardsList(cards));
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL6);
            break;
        default:
            UNIMPLIMENTEDPACKETFIELD(type);
            break;
    }
}

void BeingRecv::processBeingVisible(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    // need set type based on id
    BeingType::BeingType type = BeingType::MONSTER;
    if (msg.getVersion() >= 20091103)
    {
        msg.readInt16("len");
        type = static_cast<BeingType::BeingType>(
            msg.readUInt8("object type"));
    }

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    if (msg.getVersion() >= 20131223 &&
        (serverVersion == 0 || serverVersion >= 11))
    {
        msg.readBeingId("char id");
    }
    BeingId spawnId;
    if (id == Ea::BeingRecv::mSpawnId)
        spawnId = Ea::BeingRecv::mSpawnId;
    else
        spawnId = BeingId_zero;
    Ea::BeingRecv::mSpawnId = BeingId_zero;

    int16_t speed = msg.readInt16("speed");
    const uint32_t opt1 = msg.readInt16("opt1");
    // probably wrong effect usage
    const uint32_t opt2 = msg.readInt16("opt2");
    uint32_t option;
    if (msg.getVersion() >= 20080102)
        option = msg.readInt32("option");
    else
        option = msg.readInt16("option");
    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
    {
        dstBeing->setAction(BeingAction::SPAWN, 0);
    }
    else
    {
        dstBeing->clearPath();
        dstBeing->setActionTime(tick_time);
        dstBeing->setAction(BeingAction::STAND, 0);
    }

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(speed);
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    uint32_t weapon;
    if (msg.getVersion() >= 7)
        weapon = CAST_U32(msg.readInt32("weapon"));
    else
        weapon = CAST_U32(msg.readInt16("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");
    if (msg.getVersion() < 7)
        msg.readInt16("shield");
    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(msg.readInt16("hair color"),
        ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");

    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    if (msg.getVersion() >= 20101124)
        msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    uint32_t opt3;
    if (msg.getVersion() >= 7)
        opt3 = msg.readInt32("opt3");
    else
        opt3 = msg.readInt16("opt3");
    dstBeing->setKarma(msg.readUInt8("karma"));
    const uint8_t gender = CAST_U8(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        if (hairStyle == 0)
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR,
                0,
                std::string());
        }
        else
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR,
                hairStyle * -1,
                ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        }
        dstBeing->updateSprite(SPRITE_WEAPON, headBottom);
        dstBeing->updateSprite(SPRITE_HEAD_BOTTOM, headMid);
        dstBeing->updateSprite(SPRITE_CLOTHES_COLOR, headTop);
        dstBeing->updateSprite(SPRITE_HAIR, shoes);
        dstBeing->updateSprite(SPRITE_SHOES, gloves);
        dstBeing->updateSprite(SPRITE_BODY, weapon);
        dstBeing->setWeaponId(weapon);
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint8_t dir;
    uint16_t x, y;
    msg.readCoordinates(x, y, dir, "position");
    msg.readInt8("xs");
    msg.readInt8("ys");
    applyPlayerAction(msg, dstBeing, msg.readUInt8("action type"));
    dstBeing->setTileCoords(x, y);

    if (job == 45 && socialWindow && outfitWindow)
    {
        const int num = socialWindow->getPortalIndex(x, y);
        if (num >= 0)
        {
            dstBeing->setName(keyboard.getKeyShortString(
                outfitWindow->keyName(num)));
        }
        else
        {
            dstBeing->setName("");
        }
    }

    dstBeing->setDirection(dir);

    const int level = CAST_S32(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    if (msg.getVersion() >= 20080102)
        msg.readInt16("font");

    if (msg.getVersion() >= 20120221)
    {
        const int maxHP = msg.readInt32("max hp");
        const int hp = msg.readInt32("hp");
        dstBeing->setMaxHP(maxHP);
        dstBeing->setHP(hp);
        msg.readInt8("is boss");
    }

    if (msg.getVersion() >= 20150513)
    {
        msg.readInt16("body2");
        msg.readString(24, "name");
    }

    dstBeing->setStatusEffectOpitons(option,
        opt1,
        opt2,
        opt3);
}

void BeingRecv::processBeingMove(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    if (msg.getVersion() >= 20091103)
        msg.readInt16("len");
    BeingType::BeingType type;
    if (msg.getVersion() >= 20071106)
    {
        type = static_cast<BeingType::BeingType>(
            msg.readUInt8("object type"));
    }
    else
    {
        // need detect type based on id
        type = BeingType::MONSTER;
    }

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    if (msg.getVersion() >= 20131223 &&
        (serverVersion == 0 || serverVersion >= 11))
    {
        msg.readBeingId("char id");
    }
    BeingId spawnId;
    if (id == Ea::BeingRecv::mSpawnId)
        spawnId = Ea::BeingRecv::mSpawnId;
    else
        spawnId = BeingId_zero;
    Ea::BeingRecv::mSpawnId = BeingId_zero;
    int16_t speed = msg.readInt16("speed");
    const uint32_t opt1 = msg.readInt16("opt1");
    // probably wrong effect usage
    const uint32_t opt2 = msg.readInt16("opt2");
    uint32_t option;
    if (msg.getVersion() >= 7)
        option = msg.readInt32("option");
    else
        option = msg.readInt16("option");
    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
        dstBeing->setAction(BeingAction::SPAWN, 0);

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(speed);
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    uint32_t weapon;
    if (msg.getVersion() >= 7)
        weapon = CAST_U32(msg.readInt32("weapon"));
    else
        weapon = CAST_U32(msg.readInt16("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");
    msg.readInt32("tick");
    if (msg.getVersion() < 7)
        msg.readInt16("shield");
    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(
        msg.readInt16("hair color"), ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");

    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    if (msg.getVersion() >= 20101124)
        msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    uint32_t opt3;
    if (msg.getVersion() >= 7)
        opt3 = msg.readInt32("opt3");
    else
        opt3 = msg.readInt16("opt3");
    dstBeing->setKarma(msg.readUInt8("karma"));
    const uint8_t gender = CAST_U8(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        if (hairStyle == 0)
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR, 0);
        }
        else
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR,
                hairStyle * -1,
                ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        }
        if (!serverFeatures->haveMove3())
        {
            dstBeing->updateSprite(SPRITE_WEAPON, headBottom);
            dstBeing->updateSprite(SPRITE_HEAD_BOTTOM, headMid);
            dstBeing->updateSprite(SPRITE_CLOTHES_COLOR, headTop);
            dstBeing->updateSprite(SPRITE_HAIR, shoes);
            dstBeing->updateSprite(SPRITE_SHOES, gloves);
            dstBeing->updateSprite(SPRITE_BODY, weapon);
            dstBeing->setWeaponId(weapon);
        }
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint16_t srcX, srcY, dstX, dstY;
    msg.readCoordinatePair(srcX, srcY, dstX, dstY, "move path");
    msg.readUInt8("(sx<<4) | (sy&0x0f)");
    msg.readInt8("xs");
    msg.readInt8("ys");
    dstBeing->setAction(BeingAction::STAND, 0);
    dstBeing->setTileCoords(srcX, srcY);
    if (localPlayer)
        localPlayer->followMoveTo(dstBeing, srcX, srcY, dstX, dstY);
    if (serverFeatures->haveMove3())
        dstBeing->setCachedDestination(dstX, dstY);
    else
        dstBeing->setDestination(dstX, dstY);

    // because server don't send direction in move packet, we fixing it

    uint8_t d = 0;
    if (srcX == dstX && srcY == dstY)
    {   // if player did one step from invisible area to visible,
        // move path is broken
        int x2 = localPlayer->getTileX();
        int y2 = localPlayer->getTileY();
        if (abs(x2 - srcX) > abs(y2 - srcY))
            y2 = srcY;
        else
            x2 = srcX;
        d = dstBeing->calcDirection(x2, y2);
    }
    else
    {
        d = dstBeing->calcDirection(dstX, dstY);
    }
    if (d && dstBeing->getDirection() != d)
        dstBeing->setDirection(d);

    const int level = CAST_S32(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    if (msg.getVersion() >= 20080102)
        msg.readInt16("font");
    if (msg.getVersion() >= 20120221)
    {
        const int maxHP = msg.readInt32("max hp");
        const int hp = msg.readInt32("hp");
        dstBeing->setMaxHP(maxHP);
        dstBeing->setHP(hp);
        msg.readInt8("is boss");
    }
    if (msg.getVersion() >= 20150513)
    {
        msg.readInt16("body2");
        msg.readString(24, "name");
    }

    dstBeing->setStatusEffectOpitons(option,
        opt1,
        opt2,
        opt3);
}

void BeingRecv::processBeingSpawn(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    // need get type from id
    BeingType::BeingType type = BeingType::MONSTER;
    if (msg.getVersion() >= 20091103)
    {
        msg.readInt16("len");
        type = static_cast<BeingType::BeingType>(
            msg.readUInt8("object type"));
    }

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    if (msg.getVersion() >= 20131223 &&
        (serverVersion == 0 || serverVersion >= 11))
    {
        msg.readBeingId("char id");
    }
    Ea::BeingRecv::mSpawnId = id;
    const BeingId spawnId = id;
    int16_t speed = msg.readInt16("speed");
    const uint32_t opt1 = msg.readInt16("opt1");
    // probably wrong effect usage
    const uint32_t opt2 = msg.readInt16("opt2");
    uint32_t option;
    if (msg.getVersion() >= 20080102)
        option = msg.readInt32("option");
    else
        option = msg.readInt16("option");
    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
        dstBeing->setAction(BeingAction::SPAWN, 0);

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(speed);
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    uint32_t weapon;
    if (msg.getVersion() >= 7)
        weapon = CAST_U32(msg.readInt32("weapon"));
    else
        weapon = CAST_U32(msg.readInt16("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");
    if (msg.getVersion() < 7)
        msg.readInt16("shield");
    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(
        msg.readInt16("hair color"), ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");
    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    if (msg.getVersion() >= 20101124)
        msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    uint32_t opt3;
    if (msg.getVersion() >= 7)
        opt3 = msg.readInt32("opt3");
    else
        opt3 = msg.readInt16("opt3");

    dstBeing->setKarma(msg.readUInt8("karma"));
    const uint8_t gender = CAST_U8(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        if (hairStyle == 0)
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR, 0);
        }
        else
        {
            dstBeing->updateSprite(SPRITE_HAIR_COLOR,
                hairStyle * -1,
                ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        }
        dstBeing->updateSprite(SPRITE_WEAPON, headBottom);
        dstBeing->updateSprite(SPRITE_HEAD_BOTTOM, headMid);
        dstBeing->updateSprite(SPRITE_CLOTHES_COLOR, headTop);
        dstBeing->updateSprite(SPRITE_HAIR, shoes);
        dstBeing->updateSprite(SPRITE_SHOES, gloves);
        dstBeing->updateSprite(SPRITE_BODY, weapon);
        dstBeing->setWeaponId(weapon);
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint8_t dir;
    uint16_t x, y;
    msg.readCoordinates(x, y, dir, "position");
    msg.readInt8("xs");
    msg.readInt8("ys");
    dstBeing->setTileCoords(x, y);

    if (job == 45 && socialWindow && outfitWindow)
    {
        const int num = socialWindow->getPortalIndex(x, y);
        if (num >= 0)
        {
            dstBeing->setName(keyboard.getKeyShortString(
                outfitWindow->keyName(num)));
        }
        else
        {
            dstBeing->setName("");
        }
    }

    dstBeing->setDirection(dir);

    const int level = CAST_S32(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    if (msg.getVersion() >= 20080102)
        msg.readInt16("font");
    if (msg.getVersion() >= 20120221)
    {
        const int maxHP = msg.readInt32("max hp");
        const int hp = msg.readInt32("hp");
        dstBeing->setMaxHP(maxHP);
        dstBeing->setHP(hp);
        msg.readInt8("is boss");
    }
    if (msg.getVersion() >= 20150513)
    {
        msg.readInt16("body2");
        msg.readString(24, "name");
    }

    dstBeing->setStatusEffectOpitons(option,
        opt1,
        opt2,
        opt3);
}

void BeingRecv::processMapTypeProperty(Net::MessageIn &msg)
{
    const int16_t type = msg.readInt16("type");
    const int flags = msg.readInt32("flags");
    if (type == 0x28)
    {
        // +++ need get other flags from here
        MapTypeProperty2 props;
        props.data = CAST_U32(flags);
        const Game *const game = Game::instance();
        if (!game)
            return;
        Map *const map = game->getCurrentMap();
        if (!map)
            return;
        map->setPvpMode(props.bits.party | (props.bits.guild * 2));
    }
}

void BeingRecv::processMapType(Net::MessageIn &msg)
{
    const int16_t type = msg.readInt16("type");
    if (type == 19)
        NotifyManager::notify(NotifyTypes::MAP_TYPE_BATTLEFIELD);
    else
        UNIMPLIMENTEDPACKETFIELD(type);
}

void BeingRecv::processSkillCasting(Net::MessageIn &msg)
{
    const BeingId srcId = msg.readBeingId("src id");
    const BeingId dstId = msg.readBeingId("dst id");
    const int dstX = msg.readInt16("dst x");
    const int dstY = msg.readInt16("dst y");
    const int skillId = msg.readInt16("skill id");
    msg.readInt32("property");  // can be used to trigger effect
    const int castTime = msg.readInt32("cast time");
    if (msg.getVersion() >= 20091124)
        msg.readInt8("dispossable");

    processSkillCastingContinue(msg,
        srcId, dstId,
        dstX, dstY,
        skillId,
        1,
        0,
        SkillType2::Unknown,
        castTime);
}

void BeingRecv::processSkillCasting2(Net::MessageIn &msg)
{
    msg.readInt16("len");  // for now unused
    const BeingId srcId = msg.readBeingId("src id");
    const BeingId dstId = msg.readBeingId("dst id");
    const int dstX = msg.readInt16("dst x");
    const int dstY = msg.readInt16("dst y");
    const int skillId = msg.readInt16("skill id");
    const int skillLevel = msg.readInt16("skill level");
    msg.readInt32("property");  // can be used to trigger effect
    const int castTime = msg.readInt32("cast time");
    const int range = msg.readInt32("skill range");
    const SkillType2::SkillType2 inf2 =
        static_cast<SkillType2::SkillType2>(msg.readInt32("inf2"));

    processSkillCastingContinue(msg,
        srcId, dstId,
        dstX, dstY,
        skillId,
        skillLevel,
        range,
        inf2,
        castTime);
}

void BeingRecv::processSkillCastingContinue(Net::MessageIn &msg,
                                            const BeingId srcId,
                                            const BeingId dstId,
                                            const int dstX,
                                            const int dstY,
                                            const int skillId,
                                            const int skillLevel,
                                            const int range,
                                            const SkillType2::SkillType2 inf2,
                                            const int castTime)
{
    if (!effectManager)
        return;

    if (srcId == BeingId_zero)
    {
        UNIMPLIMENTEDPACKETFIELD(0);
        return;
    }
    Being *const srcBeing = actorManager->findBeing(srcId);
    if (dstId != BeingId_zero)
    {   // being to being
        Being *const dstBeing = actorManager->findBeing(dstId);
        if (srcBeing)
        {
            srcBeing->handleSkillCasting(dstBeing, skillId, skillLevel);
            if (dstBeing)
            {
                srcBeing->addCast(dstBeing->getTileX(),
                    dstBeing->getTileY(),
                    skillId,
                    skillLevel,
                    range,
                    castTime / MILLISECONDS_IN_A_TICK);
            }
        }
    }
    else if (dstX != 0 || dstY != 0)
    {   // being to position
        if (srcBeing)
            srcBeing->setAction(BeingAction::CAST, skillId);
        skillDialog->playCastingDstTileEffect(skillId,
            skillLevel,
            dstX, dstY,
            castTime);
        if (srcBeing)
        {
            srcBeing->addCast(dstX, dstY,
                skillId,
                skillLevel,
                range,
                castTime / MILLISECONDS_IN_A_TICK);
        }
    }
    if (srcBeing == localPlayer &&
        (inf2 & SkillType2::FreeCastAny) == 0)
    {
        localPlayer->freezeMoving(castTime / MILLISECONDS_IN_A_TICK);
    }
}

void BeingRecv::processBeingStatusChange(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingStatusChange")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingStatusChange")
        return;
    }

    // Status change
    const uint16_t status = msg.readInt16("status");
    const BeingId id = msg.readBeingId("being id");
    const Enable flag = fromBool(
        msg.readUInt8("flag: 0: stop, 1: start"), Enable);
    if (msg.getVersion() >= 20120618)
        msg.readInt32("total");
    if (msg.getVersion() >= 20090121)
    {
        msg.readInt32("left");
        msg.readInt32("val1");
        msg.readInt32("val2");
        msg.readInt32("val3");
    }

    const IsStart start = msg.getVersion() == 20090121 ?
        IsStart_false : IsStart_true;

    Being *const dstBeing = actorManager->findBeing(id);
    if (dstBeing)
        dstBeing->setStatusEffect(status, flag, start);
    BLOCK_END("BeingRecv::processBeingStatusChange")
}

void BeingRecv::processBeingMove2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingMove2")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingMove2")
        return;
    }

    /*
      * A simplified movement packet, used by the
      * later versions of eAthena for both mobs and
      * players
      */
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));

    uint16_t srcX, srcY, dstX, dstY;
    msg.readCoordinatePair(srcX, srcY, dstX, dstY, "move path");
    msg.readUInt8("(sx<<4) | (sy&0x0f)");
    msg.readInt32("tick");

    /*
      * This packet doesn't have enough info to actually
      * create a new being, so if the being isn't found,
      * we'll just pretend the packet didn't happen
      */

    if (!dstBeing)
    {
        BLOCK_END("BeingRecv::processBeingMove2")
        return;
    }

    dstBeing->setTileCoords(srcX, srcY);
    if (localPlayer)
        localPlayer->followMoveTo(dstBeing, srcX, srcY, dstX, dstY);
    if (serverFeatures->haveMove3())
        dstBeing->setCachedDestination(dstX, dstY);
    else
        dstBeing->setDestination(dstX, dstY);
    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();
    BLOCK_END("BeingRecv::processBeingMove2")
}

void BeingRecv::processBeingAction2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingAction2")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingAction2")
        return;
    }

    Being *const srcBeing = actorManager->findBeing(
        msg.readBeingId("src being id"));
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("dst being id"));

    msg.readInt32("tick");
    const int srcSpeed = msg.readInt32("src speed");
    msg.readInt32("dst speed");
    int param1;
    if (msg.getVersion() >= 20071113)
        param1 = msg.readInt32("damage");
    else
        param1 = msg.readInt16("damage");
    if (msg.getVersion() >= 20131223)
        msg.readUInt8("is sp damaged");
    msg.readInt16("count");
    const AttackTypeT type = static_cast<AttackTypeT>(
        msg.readUInt8("action"));
    if (msg.getVersion() >= 20071113)
        msg.readInt32("left damage");
    else
        msg.readInt16("left damage");

    switch (type)
    {
        case AttackType::HIT:  // Damage
        case AttackType::CRITICAL:  // Critical Damage
        case AttackType::MULTI:  // Critical Damage
        case AttackType::MULTI_REFLECT:
        case AttackType::REFLECT:  // Reflected Damage
        case AttackType::FLEE:  // Lucky Dodge
        case AttackType::SPLASH:
        case AttackType::SKILL:
        case AttackType::REPEATE:
            if (srcBeing)
            {
                if (srcSpeed && srcBeing->getType() == ActorType::Player)
                    srcBeing->setAttackDelay(srcSpeed);
                // attackid=1, type
                srcBeing->handleAttack(dstBeing, param1, 1);
                if (srcBeing->getType() == ActorType::Player)
                    srcBeing->setAttackTime();
            }
            if (dstBeing)
            {
                // level not present, using 1
                dstBeing->takeDamage(srcBeing, param1,
                    static_cast<AttackTypeT>(type), 1);
            }
            break;

        case AttackType::PICKUP:
            break;

        case AttackType::TOUCH_SKILL:
            break;

        case AttackType::SIT:
            if (srcBeing)
            {
                srcBeing->setAction(BeingAction::SIT, 0);
                if (srcBeing->getType() == ActorType::Player)
                {
                    srcBeing->setMoveTime();
                    if (localPlayer)
                        localPlayer->imitateAction(srcBeing, BeingAction::SIT);
                }
            }
            break;

        case AttackType::STAND:
            if (srcBeing)
            {
                srcBeing->setAction(BeingAction::STAND, 0);
                if (srcBeing->getType() == ActorType::Player)
                {
                    srcBeing->setMoveTime();
                    if (localPlayer)
                    {
                        localPlayer->imitateAction(srcBeing,
                            BeingAction::STAND);
                    }
                }
            }
            break;
        default:
        case AttackType::MISS:
        case AttackType::SKILLMISS:
            UNIMPLIMENTEDPACKETFIELD(CAST_S32(type));
            break;
    }
    BLOCK_END("BeingRecv::processBeingAction2")
}

void BeingRecv::processBeingHp(Net::MessageIn &msg)
{
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));
    int hp;
    int maxHP;
    if (msg.getVersion() >= 20100126)
    {
        hp = msg.readInt32("hp");
        maxHP = msg.readInt32("max hp");
    }
    else
    {
        hp = msg.readInt16("hp");
        maxHP = msg.readInt16("max hp");
    }
    if (dstBeing)
    {
        dstBeing->setHP(hp);
        dstBeing->setMaxHP(maxHP);
    }
}

void BeingRecv::processMonsterHp(Net::MessageIn &msg)
{
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("monster id"));
    const int hp = msg.readInt32("hp");
    const int maxHP = msg.readInt32("max hp");
    if (dstBeing)
    {
        dstBeing->setHP(hp);
        dstBeing->setMaxHP(maxHP);
    }
}

void BeingRecv::processSkillAutoCast(Net::MessageIn &msg)
{
    const int id = msg.readInt16("skill id");
    msg.readInt16("inf");
    msg.readInt16("unused");
    const int level = msg.readInt16("skill level");
    msg.readInt16("sp");
    msg.readInt16("range");
    msg.readString(24, "skill name");
    msg.readInt8("unused");

    if (localPlayer)
    {
        localPlayer->handleSkill(localPlayer, 0, id, level);
        localPlayer->takeDamage(localPlayer, 0, AttackType::SKILL, id, level);
    }
}

void BeingRecv::processRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    msg.readInt16("rank type");
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
    msg.readInt32("my points");
}

void BeingRecv::processBlacksmithRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processAlchemistRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processTaekwonRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processPkRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processBeingChangeDirection(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingChangeDirection")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingChangeDirection")
        return;
    }

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));

    msg.readInt16("head direction");

    const uint8_t dir = Net::MessageIn::fromServerDirection(
        CAST_U8(msg.readUInt8("player direction") & 0x0FU));

    if (!dstBeing)
    {
        BLOCK_END("BeingRecv::processBeingChangeDirection")
        return;
    }

    dstBeing->setDirection(dir);
    if (localPlayer)
        localPlayer->imitateDirection(dstBeing, dir);
    BLOCK_END("BeingRecv::processBeingChangeDirection")
}

void BeingRecv::processBeingSpecialEffect(Net::MessageIn &msg)
{
    if (!effectManager || !actorManager)
        return;

    const BeingId id = msg.readBeingId("being id");
    Being *const being = actorManager->findBeing(id);
    if (!being)
        return;

    const int effectType = msg.readInt32("effect type");

    if (ParticleEngine::enabled)
        effectManager->trigger(effectType, being);

    // +++ need dehard code effectType == 3
    if (effectType == 3 && being->getType() == ActorType::Player
        && socialWindow)
    {   // reset received damage
        socialWindow->resetDamage(being->getName());
    }
}

void BeingRecv::processBeingSpecialEffectNum(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ need somhow show this effects.
    // type is not same with self/misc effect.
    msg.readBeingId("account id");
    msg.readInt32("effect type");
    msg.readInt32("num");  // effect variable
}

void BeingRecv::processBeingSoundEffect(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ need play this effect.
    msg.readString(24, "sound effect name");
    msg.readUInt8("type");
    msg.readInt32("unused");
    msg.readInt32("source being id");
}

void BeingRecv::processSkillGroundNoDamage(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("skill id");
    msg.readInt32("src id");
    msg.readInt16("val");
    msg.readInt16("x");
    msg.readInt16("y");
    msg.readInt32("tick");
}

void BeingRecv::processSkillEntry(Net::MessageIn &msg)
{
    if (msg.getVersion() >= 20110718)
        msg.readInt16("len");
    const BeingId id = msg.readBeingId("skill unit id");
    const BeingId creatorId = msg.readBeingId("creator accound id");
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    int job = 0;
    if (msg.getVersion() >= 20121212)
        job = msg.readInt32("job");
    if (msg.getVersion() >= 20110718)
        msg.readUInt8("radius");
    msg.readUInt8("visible");
    int level = 0;
    if (msg.getVersion() >= 20130731)
        level = msg.readUInt8("level");
    Being *const dstBeing = createBeing2(msg,
        id,
        job,
        BeingType::SKILL);
    if (!dstBeing)
        return;
    dstBeing->setAction(BeingAction::STAND, 0);
    dstBeing->setTileCoords(x, y);
    dstBeing->setLevel(level);
    dstBeing->setCreatorId(creatorId);
}

void BeingRecv::processPlayerStatusChange(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPlayerStop")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processPlayerStop")
        return;
    }

    // Change in players' flags
    const BeingId id = msg.readBeingId("account id");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    const uint32_t opt1 = msg.readInt16("opt1");
    const uint32_t opt2 = msg.readInt16("opt2");
    uint32_t option;
    if (msg.getVersion() >= 7)
        option = msg.readInt32("option");
    else
        option = msg.readInt16("option");
    dstBeing->setKarma(msg.readUInt8("karma"));

    dstBeing->setStatusEffectOpitons(option,
        opt1,
        opt2);
    BLOCK_END("BeingRecv::processPlayerStop")
}

void BeingRecv::processPlayerStatusChange2(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    // look like this function unused on server

    const BeingId id = msg.readBeingId("account id");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    const uint32_t option = msg.readInt32("option");
    dstBeing->setLevel(msg.readInt32("level"));
    msg.readInt32("showEFST");
    dstBeing->setStatusEffectOpiton0(option);
}

void BeingRecv::processBeingResurrect(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingResurrect")
    if (!actorManager || !localPlayer)
    {
        BLOCK_END("BeingRecv::processBeingResurrect")
        return;
    }

    // A being changed mortality status

    const BeingId id = msg.readBeingId("being id");
    msg.readInt16("unused");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
    {
        DEBUGLOGSTR("insible player?");
        BLOCK_END("BeingRecv::processBeingResurrect")
        return;
    }

    // If this is player's current target, clear it.
    if (dstBeing == localPlayer->getTarget())
        localPlayer->stopAttack();

    dstBeing->setAction(BeingAction::STAND, 0);
    BLOCK_END("BeingRecv::processBeingResurrect")
}

void BeingRecv::processPlayerGuilPartyInfo(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPlayerGuilPartyInfo")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processPlayerGuilPartyInfo")
        return;
    }

    const BeingId beingId = msg.readBeingId("being id");
    const std::string name = msg.readString(24, "char name");
    actorManager->updateNameId(name, beingId);
    Being *const dstBeing = actorManager->findBeing(beingId);
    if (dstBeing)
    {
        if (beingId == localPlayer->getId())
        {
            localPlayer->pingResponse();
        }
        dstBeing->setName(name);
        dstBeing->setPartyName(msg.readString(24, "party name"));
        dstBeing->setGuildName(msg.readString(24, "guild name"));
        dstBeing->setGuildPos(msg.readString(24, "guild pos"));
        dstBeing->addToCache();
    }
    else
    {
        msg.readString(24, "party name");
        msg.readString(24, "guild name");
        msg.readString(24, "guild pos");
    }
    BLOCK_END("BeingRecv::processPlayerGuilPartyInfo")
}

void BeingRecv::processBeingRemoveSkill(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("skill unit id");
    if (!actorManager)
        return;
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;
    actorManager->destroy(dstBeing);
}

void BeingRecv::processBeingFakeName(Net::MessageIn &msg)
{
    uint16_t x, y;
    uint8_t dir;
    if (msg.getVersion() < 20071106)
    {
        msg.readBeingId("npc id");
        msg.skip(8, "unused");
        msg.readInt16("class?");  // 111
        msg.skip(30, "unused");
        msg.readCoordinates(x, y, dir, "position");
        msg.readUInt8("sx");
        msg.readUInt8("sy");
        msg.skip(3, "unused");
        return;
    }
    const BeingType::BeingType type = static_cast<BeingType::BeingType>(
        msg.readUInt8("object type"));
    const BeingId id = msg.readBeingId("npc id");
    msg.skip(8, "unused");
    const uint16_t job = msg.readInt16("class?");  // 111
    msg.skip(30, "unused");
    msg.readCoordinates(x, y, dir, "position");
    msg.readUInt8("sx");
    msg.readUInt8("sy");
    msg.skip(3, "unused");

    Being *const dstBeing = createBeing2(msg, id, job, type);
    if (!dstBeing)
        return;
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    dstBeing->setTileCoords(x, y);
    dstBeing->setDirection(dir);
}

void BeingRecv::processBeingStatUpdate1(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("account id");
    const int type = msg.readInt16("type");
    const int value = msg.readInt32("value");

    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    if (type != Sp::MANNER)
    {
        UNIMPLIMENTEDPACKETFIELD(type);
        return;
    }
    dstBeing->setManner(value);
}

void BeingRecv::processBeingSelfEffect(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingSelfEffect")
    if (!effectManager || !actorManager)
    {
        BLOCK_END("BeingRecv::processBeingSelfEffect")
        return;
    }

    const BeingId id = msg.readBeingId("being id");
    Being *const being = actorManager->findBeing(id);
    if (!being)
    {
        DEBUGLOGSTR("insible player?");
        msg.readInt32("effect type");
        BLOCK_END("BeingRecv::processBeingSelfEffect")
        return;
    }

    const int effectType = msg.readInt32("effect type");
    if (ParticleEngine::enabled)
        effectManager->trigger(effectType, being);

    BLOCK_END("BeingRecv::processBeingSelfEffect")
}

void BeingRecv::processMobInfo(Net::MessageIn &msg)
{
    const int len = msg.readInt16("len");
    if (len < 12)
        return;
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("monster id"));
    const int attackRange = msg.readInt32("range");
    if (dstBeing)
        dstBeing->setAttackRange(attackRange);
}

void BeingRecv::processBeingAttrs(Net::MessageIn &msg)
{
    const int len = msg.readInt16("len");
    if (len < 12)
        return;
    const bool haveMount = serverFeatures->haveExtendedRiding();
    if (haveMount && len < 14)
        return;

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("player id"));
    const int gmLevel = msg.readInt32("gm level");
    uint16_t mount = 0;
    if (haveMount)
        mount = msg.readInt16("mount");
    if (dstBeing)
    {
        if (gmLevel)
        {
            if (dstBeing == localPlayer)
                localPlayer->setGMLevel(gmLevel);
            dstBeing->setGM(true);
        }
        else
        {
            if (dstBeing == localPlayer)
                localPlayer->setGMLevel(0);
            dstBeing->setGM(false);
        }
        if (haveMount)
        {
            dstBeing->setHorse(mount);
        }
    }
}

void BeingRecv::processMonsterInfo(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readInt16("class");
    msg.readInt16("level");
    msg.readInt16("size");
    msg.readInt32("hp");
    msg.readInt16("def");
    msg.readInt16("race");
    msg.readInt16("mdef");
    msg.readInt16("ele");
}

void BeingRecv::processClassChange(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readUInt8("type");
    msg.readInt32("class");
}

void BeingRecv::processSpiritBalls(Net::MessageIn &msg)
{
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));
    const int balls = msg.readInt16("spirits amount");
    if (dstBeing)
        dstBeing->setSpiritBalls(balls);
}

void BeingRecv::processBladeStop(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readInt32("src being id");
    msg.readInt32("dst being id");
    msg.readInt32("flag");
}

void BeingRecv::processComboDelay(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readInt32("wait");
}

void BeingRecv::processWddingEffect(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
}

void BeingRecv::processBeingSlide(Net::MessageIn &msg)
{
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    if (!dstBeing)
        return;
    if (localPlayer == dstBeing)
    {
        localPlayer->stopAttack();
        localPlayer->navigateClean();
        if (viewport)
            viewport->returnCamera();
    }

    dstBeing->setAction(BeingAction::STAND, 0);
    dstBeing->setTileCoords(x, y);
}

void BeingRecv::processStarsKill(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readString(24, "map name");
    msg.readInt32("monster id");
    msg.readUInt8("start");
    msg.readUInt8("result");
}

void BeingRecv::processGladiatorFeelRequest(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readUInt8("which");
}

void BeingRecv::processBossMapInfo(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readUInt8("info type");
    msg.readInt32("x");
    msg.readInt32("y");
    msg.readInt16("min hours");
    msg.readInt16("min minutes");
    msg.readInt16("max hours");
    msg.readInt16("max minutes");
    msg.readString(24, "monster name");  // really can be used 51 byte?
}

void BeingRecv::processBeingFont(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("font");
}

void BeingRecv::processBeingMilleniumShield(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("shields");
    msg.readInt16("unused");
}

void BeingRecv::processBeingCharm(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("charm type");
    msg.readInt16("charm count");
}

void BeingRecv::processBeingViewEquipment(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    const int count = (msg.readInt16("len") - 45) / 31;
    msg.readString(24, "name");
    msg.readInt16("job");
    msg.readInt16("head");
    msg.readInt16("accessory");
    msg.readInt16("accessory2");
    msg.readInt16("accessory3");
    if (msg.getVersion() >= 20101124)
        msg.readInt16("robe");
    msg.readInt16("hair color");
    msg.readInt16("body color");
    msg.readUInt8("gender");
    for (int f = 0; f < count; f ++)
    {
        msg.readInt16("index");
        msg.readInt16("item id");
        msg.readUInt8("item type");
        msg.readInt32("location");
        msg.readInt32("wear state");
        msg.readInt8("refine");
        for (int d = 0; d < maxCards; d ++)
            msg.readUInt16("card");
        msg.readInt32("hire expire date (?)");
        msg.readInt16("equip type");
        msg.readInt16("item sprite number");
        msg.readUInt8("flags");
    }
}

void BeingRecv::processPvpSet(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPvpSet")
    const BeingId id = msg.readBeingId("being id");
    const int rank = msg.readInt32("rank");
    msg.readInt32("num");
    if (actorManager)
    {
        Being *const dstBeing = actorManager->findBeing(id);
        if (dstBeing)
            dstBeing->setPvpRank(rank);
    }
    BLOCK_END("BeingRecv::processPvpSet")
}

void BeingRecv::processNameResponse2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processNameResponse2")
    if (!actorManager || !localPlayer)
    {
        BLOCK_END("BeingRecv::processNameResponse2")
        return;
    }

    const int len = msg.readInt16("len");
    const BeingId beingId = msg.readBeingId("account id");
    const std::string str = msg.readString(len - 8, "name");
    actorManager->updateNameId(str, beingId);
    Being *const dstBeing = actorManager->findBeing(beingId);
    if (dstBeing)
    {
        if (beingId == localPlayer->getId())
        {
            localPlayer->pingResponse();
        }
        else
        {
            dstBeing->setName(str);
            dstBeing->updateGuild();
            dstBeing->addToCache();

            if (dstBeing->getType() == ActorType::Player)
                dstBeing->updateColors();

            if (localPlayer)
            {
                const Party *const party = localPlayer->getParty();
                if (party && party->isMember(dstBeing->getId()))
                {
                    PartyMember *const member = party->getMember(
                        dstBeing->getId());

                    if (member)
                        member->setName(dstBeing->getName());
                }
                localPlayer->checkNewName(dstBeing);
            }
        }
    }
    BLOCK_END("BeingRecv::processNameResponse2")
}

Being *BeingRecv::createBeing2(Net::MessageIn &msg,
                               const BeingId id,
                               const int32_t job,
                               const BeingType::BeingType beingType)
{
    if (!actorManager)
        return nullptr;

    ActorTypeT type = ActorType::Unknown;
    switch (beingType)
    {
        case BeingType::PC:
            type = ActorType::Player;
            break;
        case BeingType::NPC:
        case BeingType::NPC_EVENT:
            type = ActorType::Npc;
            break;
        case BeingType::MONSTER:
            type = ActorType::Monster;
            break;
        case BeingType::MERSOL:
            type = ActorType::Mercenary;
            break;
        case BeingType::PET:
            type = ActorType::Pet;
            break;
        case BeingType::HOMUN:
            type = ActorType::Homunculus;
            break;
        case BeingType::SKILL:
            type = ActorType::SkillUnit;
            break;
        case BeingType::ITEM:
        case BeingType::ELEMENTAL:
            logger->log("not supported object type: %d, job: %d",
                CAST_S32(beingType), CAST_S32(job));
            break;
        case BeingType::CHAT:
        default:
            UNIMPLIMENTEDPACKETFIELD(beingType);
            type = ActorType::Monster;
            logger->log("not supported object type: %d, job: %d",
                CAST_S32(beingType), CAST_S32(job));
            break;
    }
    if (job == 45 && beingType == BeingType::NPC_EVENT)
        type = ActorType::Portal;

    Being *const being = actorManager->createBeing(
        id, type, fromInt(job, BeingTypeId));
    if (beingType == BeingType::MERSOL)
    {
        const MercenaryInfo *const info = PlayerInfo::getMercenary();
        if (info && info->id == id)
            PlayerInfo::setMercenaryBeing(being);
    }
    else if (beingType == BeingType::PET)
    {
        if (PlayerInfo::getPetBeingId() == id)
            PlayerInfo::setPetBeing(being);
    }
    return being;
}

void BeingRecv::processSkillCancel(Net::MessageIn &msg)
{
    msg.readInt32("id?");
}

void BeingRecv::processSolveCharName(Net::MessageIn &msg)
{
    const int id = msg.readInt32("char id");
    const std::string name = msg.readString(24, "name");
    if (actorManager)
        actorManager->addChar(id, name);
}

void BeingRecv::processGraffiti(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("graffiti id");
    const BeingId creatorId = msg.readBeingId("creator id");
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    const int job = msg.readUInt8("job");
    msg.readUInt8("visible");
    msg.readUInt8("is content");
    const std::string text = msg.readString(80, "text");

    Being *const dstBeing = createBeing2(msg, id, job, BeingType::SKILL);
    if (!dstBeing)
        return;

    dstBeing->setAction(BeingAction::STAND, 0);
    dstBeing->setTileCoords(x, y);
    dstBeing->setShowName(true);
    dstBeing->setName(text);
    dstBeing->setCreatorId(creatorId);
}

void BeingRecv::processSkillDamage(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processSkillDamage")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processSkillDamage")
        return;
    }

    const int id = msg.readInt16("skill id");
    Being *const srcBeing = actorManager->findBeing(
        msg.readBeingId("src being id"));
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("dst being id"));
    msg.readInt32("tick");
    msg.readInt32("src speed");
    msg.readInt32("dst speed");
    int param1;
    if (msg.getVersion() >= 3)
        param1 = msg.readInt32("damage");
    else
        param1 = msg.readInt16("damage");
    const int level = msg.readInt16("skill level");
    msg.readInt16("div");
    msg.readUInt8("skill hit/type?");
    if (srcBeing)
        srcBeing->handleSkill(dstBeing, param1, id, level);
    if (dstBeing)
        dstBeing->takeDamage(srcBeing, param1, AttackType::SKILL, id, level);
    BLOCK_END("BeingRecv::processSkillDamage")
}

void BeingRecv::processNavigateTo(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // 0 position
    // 1 no position
    // 3 monster
    msg.readUInt8("navigate type");
    msg.readUInt8("transportation flag");
    msg.readUInt8("hide window");
    msg.readString(16, "map name");
    msg.readInt16("x");
    msg.readInt16("y");
    msg.readInt16("mob id");
}

void BeingRecv::applyPlayerAction(Net::MessageIn &msg,
                                  Being *const being,
                                  const uint8_t type)
{
    if (!being)
        return;
    switch (type)
    {
        case 0:
            being->setAction(BeingAction::STAND, 0);
            localPlayer->imitateAction(being, BeingAction::STAND);
            break;

        case 1:
            if (being->getCurrentAction() != BeingAction::DEAD)
            {
                being->setAction(BeingAction::DEAD, 0);
                being->recalcSpritesOrder();
            }
            break;

        case 2:
            being->setAction(BeingAction::SIT, 0);
            localPlayer->imitateAction(being, BeingAction::SIT);
            break;

        default:
            UNIMPLIMENTEDPACKETFIELD(type);
            break;
    }
}

}  // namespace EAthena
