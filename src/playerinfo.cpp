/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "playerinfo.h"

#include "client.h"
#include "depricatedevent.h"
#include "equipment.h"
#include "inventory.h"
#include "listener.h"
#include "logger.h"

#include "resources/itemdb.h"
#include "resources/iteminfo.h"

#include "net/net.h"
#include "net/playerhandler.h"

#include "debug.h"

namespace PlayerInfo
{

class PlayerInfoListener;

PlayerInfoListener *mListener = nullptr;

PlayerInfoBackend mData;
int mCharId = 0;

Inventory *mInventory = nullptr;
Equipment *mEquipment = nullptr;

std::map<int, Special> mSpecials;
char mSpecialRechargeUpdateNeeded = 0;

bool mTrading = false;
int mLevelProgress = 0;

// --- Triggers ---------------------------------------------------------------

void triggerAttr(int id, int old)
{
    DepricatedEvent event(EVENT_UPDATEATTRIBUTE);
    event.setInt("id", id);
    event.setInt("oldValue", old);
    event.setInt("newValue", mData.mAttributes.find(id)->second);
    DepricatedEvent::trigger(CHANNEL_ATTRIBUTES, event);
}

void triggerStat(int id, const std::string &changed, int old1, int old2)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it == mData.mStats.end())
        return;

    DepricatedEvent event(EVENT_UPDATESTAT);
    event.setInt("id", id);
    const Stat &stat = it->second;
    event.setInt("base", stat.base);
    event.setInt("mod", stat.mod);
    event.setInt("exp", stat.exp);
    event.setInt("expNeeded", stat.expNeed);
    event.setString("changed", changed);
    event.setInt("oldValue1", old1);
    event.setInt("oldValue2", old2);
    DepricatedEvent::trigger(CHANNEL_ATTRIBUTES, event);
}

// --- Attributes -------------------------------------------------------------

int getAttribute(int id)
{
    IntMap::const_iterator it = mData.mAttributes.find(id);
    if (it != mData.mAttributes.end())
        return it->second;
    else
        return 0;
}

void setAttribute(int id, int value, bool notify)
{
    int old = mData.mAttributes[id];
    mData.mAttributes[id] = value;
    if (notify)
        triggerAttr(id, old);
}

// --- Stats ------------------------------------------------------------------

int getStatBase(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base;
    else
        return 0;
}

void setStatBase(int id, int value, bool notify)
{
    int old = mData.mStats[id].base;
    mData.mStats[id].base = value;
    if (notify)
        triggerStat(id, "base", old);
}

int getStatMod(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.mod;
    else
        return 0;
}

void setStatMod(int id, int value, bool notify)
{
    int old = mData.mStats[id].mod;
    mData.mStats[id].mod = value;
    if (notify)
        triggerStat(id, "mod", old);
}

int getStatEffective(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    if (it != mData.mStats.end())
        return it->second.base + it->second.mod;
    else
        return 0;
}

std::pair<int, int> getStatExperience(int id)
{
    StatMap::const_iterator it = mData.mStats.find(id);
    int a, b;
    if (it != mData.mStats.end())
    {
        a = it->second.exp;
        b = it->second.expNeed;
    }
    else
    {
        a = 0;
        b = 0;
    }
    return std::pair<int, int>(a, b);
}

void setStatExperience(int id, int have, int need, bool notify)
{
    Stat &stat = mData.mStats[id];

    int oldExp = stat.exp;
    int oldExpNeed = stat.expNeed;
    stat.exp = have;
    stat.expNeed = need;
    if (notify)
        triggerStat(id, "exp", oldExp, oldExpNeed);
}

// --- Inventory / Equipment --------------------------------------------------

Inventory *getInventory()
{
    return mInventory;
}

void clearInventory()
{
    if (mEquipment)
        mEquipment->clear();
    if (mInventory)
        mInventory->clear();
}

void setInventoryItem(int index, int id, int amount, int refine)
{
    bool equipment = false;
    int itemType = ItemDB::get(id).getType();
    if (itemType != ITEM_UNUSABLE && itemType != ITEM_USABLE)
        equipment = true;
    if (mInventory)
        mInventory->setItem(index, id, amount, refine, equipment);
}

Equipment *getEquipment()
{
    return mEquipment;
}

Item *getEquipment(unsigned int slot)
{
    if (mEquipment)
        return mEquipment->getEquipment(slot);
    else
        return nullptr;
}

void setEquipmentBackend(Equipment::Backend *backend)
{
    if (mEquipment)
        mEquipment->setBackend(backend);
}

// --- Specials ---------------------------------------------------------------

void setSpecialStatus(int id, int current, int max, int recharge)
{
    logger->log("SpecialUpdate Skill #%d -- (%d/%d) -> %d", id, current, max,
                recharge);
    mSpecials[id].currentMana = current;
    mSpecials[id].neededMana = max;
    mSpecials[id].recharge = recharge;
}

const SpecialsMap &getSpecialStatus()
{
    return mSpecials;
}

// --- Misc -------------------------------------------------------------------

void setBackend(const PlayerInfoBackend &backend)
{
    mData = backend;
}

void setCharId(int charId)
{
    mCharId = charId;
}

int getCharId()
{
    return mCharId;
}

void logic()
{
    if ((mSpecialRechargeUpdateNeeded % 11) == 0)
    {
        mSpecialRechargeUpdateNeeded = 0;
        for (SpecialsMap::iterator it = mSpecials.begin(),
             it_end = mSpecials.end(); it != it_end; ++it)
        {
            it->second.currentMana += it->second.recharge;
            if (it->second.currentMana > it->second.neededMana)
                it->second.currentMana = it->second.neededMana;
        }
    }
    mSpecialRechargeUpdateNeeded++;
}

bool isTrading()
{
    return mTrading;
}

void setTrading(bool trading)
{
    bool notify = mTrading != trading;
    mTrading = trading;

    if (notify)
    {
        DepricatedEvent event(EVENT_TRADING);
        event.setInt("trading", trading);
        DepricatedEvent::trigger(CHANNEL_STATUS, event);
    }
}

void updateAttrs()
{
    int attr = Net::getPlayerHandler()->getAttackLocation();
    if (attr != -1 && getStatBase(ATTACK_DELAY))
    {
        setStatBase(ATTACK_SPEED, getStatBase(attr) * 1000 / getStatBase(
            ATTACK_DELAY), false);
        setStatMod(ATTACK_SPEED, getStatMod(attr) * 1000 / getStatBase(
            ATTACK_DELAY), true);
    }
    else
    {
        setStatBase(ATTACK_SPEED, 0, false);
        setStatMod(ATTACK_SPEED, 0, true);
    }
}

class PlayerInfoListener : Listener
{
public:
    PlayerInfoListener()
    {
        listen(CHANNEL_CLIENT);
        listen(CHANNEL_GAME);
    }

    void processEvent(Channels channel, const DepricatedEvent &event)
    {
        if (channel == CHANNEL_CLIENT)
        {
            if (event.getName() == EVENT_STATECHANGE)
            {
                int newState = event.getInt("newState");

                if (newState == STATE_GAME)
                {
                    if (!mInventory)
                    {
                        mInventory = new Inventory(Inventory::INVENTORY);
                        mEquipment = new Equipment();
                    }
                }
            }
        }
        else if (channel == CHANNEL_GAME)
        {
            if (event.getName() == EVENT_DESTRUCTED)
            {
                delete mInventory;
                mInventory = nullptr;
                delete mEquipment;
                mEquipment = nullptr;
            }
        }
    }
};

void init()
{
    if (mListener)
        return;

    // may be need remove it?
    mListener = new PlayerInfoListener();
}

void deinit()
{
    delete mListener;
    mListener = nullptr;
}

} // namespace PlayerInfo
