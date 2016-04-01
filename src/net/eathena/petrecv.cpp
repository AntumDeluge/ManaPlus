/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2016  The ManaPlus Developers
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

#include "net/eathena/petrecv.h"

#include "actormanager.h"
#include "notifymanager.h"

#include "being/localplayer.h"
#include "being/petinfo.h"
#include "being/playerinfo.h"

#include "const/net/inventory.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/eggselectiondialog.h"

#include "gui/widgets/createwidget.h"

#include "gui/widgets/tabs/chat/chattab.h"

#include "net/inventoryhandler.h"
#include "net/messagein.h"

#include "net/eathena/menu.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "debug.h"

namespace EAthena
{

void PetRecv::processPetMessage(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("pet id");
    const int data = msg.readInt32("param");
    const Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    const int hungry = data - (toInt(dstBeing->getSubType(), int)
        - 100) * 100 - 50;
    if (hungry >= 0 && hungry <= 4)
    {
        if (localChatTab && localPlayer)
        {
            // TRANSLATORS: user's pet
            std::string nick = strprintf(_("%s's pet"),
                localPlayer->getName().c_str());
            localChatTab->chatLog(nick,
                // TRANSLATORS: pet hungry level
                strprintf(_("hungry level %d"), hungry));
        }
        PetInfo *const info = PlayerInfo::getPet();
        if (!info || info->id != id)
            return;
        info->hungry = hungry;
    }
}

void PetRecv::processPetRoulette(Net::MessageIn &msg)
{
    const uint8_t data = msg.readUInt8("data");
    switch (data)
    {
        case 0:
            NotifyManager::notify(NotifyTypes::PET_CATCH_FAILED);
            break;
        case 1:
            NotifyManager::notify(NotifyTypes::PET_CATCH_SUCCESS);
            break;
        default:
            NotifyManager::notify(NotifyTypes::PET_CATCH_UNKNOWN, data);
            break;
    }
}

void PetRecv::processEggsList(Net::MessageIn &msg)
{
    const int count = (msg.readInt16("len") - 4) / 2;
    const Inventory *const inv = PlayerInfo::getInventory();
    if (!inv)
        return;
    menu = MenuType::Eggs;

    if (count == 1)
    {
        const int index = msg.readInt16("index") - INVENTORY_OFFSET;
        const Item *const item = inv->getItem(index);
        inventoryHandler->selectEgg(item);
        return;
    }
    SellDialog *const dialog = CREATEWIDGETR0(EggSelectionDialog);

    for (int f = 0; f < count; f ++)
    {
        const int index = msg.readInt16("index") - INVENTORY_OFFSET;
        const Item *const item = inv->getItem(index);

        if (item)
            dialog->addItem(item, 0);
    }
}

void PetRecv::processPetData(Net::MessageIn &msg)
{
    const int cmd = msg.readUInt8("type");
    const BeingId id = msg.readBeingId("pet id");
    Being *const dstBeing = actorManager->findBeing(id);
    const int data = msg.readInt32("data");
    if (!cmd)  // pre init
    {
        PetInfo *const info = new PetInfo;
        info->id = id;
        PlayerInfo::setPet(info);
        PlayerInfo::setPetBeing(dstBeing);
        return;
    }
    PetInfo *const info = PlayerInfo::getPet();
    if (!info)
        return;
    switch (cmd)
    {
        case 1:  // intimacy
            info->intimacy = data;
            break;
        case 2:  // hunger
            info->hungry = data;
            break;
        case 3:  // accesory
            info->equip = data;
            break;
        case 4:  // performance
            info->performance = data;
            break;
        case 5:  // hair style
            info->hairStyle = data;
            break;
        default:
            break;
    }
}

void PetRecv::processPetStatus(Net::MessageIn &msg)
{
    const std::string name = msg.readString(24, "pet name");
    msg.readUInt8("rename flag");
    const int level = msg.readInt16("level");
    const int hungry = msg.readInt16("hungry");
    const int intimacy = msg.readInt16("intimacy");
    const int equip = msg.readInt16("equip");
    const int race = msg.readInt16("class");

//    Being *const being = PlayerInfo::getPetBeing();
//    if (being)
//        being->setLevel(level);

    PetInfo *const info = PlayerInfo::getPet();
    if (!info)
        return;
    info->name = name;
    info->level = level;
    info->hungry = hungry;
    info->intimacy = intimacy;
    info->equip = equip;
    if (msg.getVersion() >= 20081126)
        info->race = race;
    else
        info->race = 0;
}

void PetRecv::processPetFood(Net::MessageIn &msg)
{
    const int result = msg.readUInt8("result");
    msg.readInt16("food id");
    if (result)
        NotifyManager::notify(NotifyTypes::PET_FEED_OK);
    else
        NotifyManager::notify(NotifyTypes::PET_FEED_ERROR);
}

void PetRecv::processPetCatchProcess(Net::MessageIn &msg A_UNUSED)
{
    NotifyManager::notify(NotifyTypes::PET_CATCH_PROCESS);
}

}  // namespace EAthena
