/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/tabs/chat/partytab.h"

#include "chatlogger.h"
#include "configuration.h"
#include "party.h"
#include "soundconsts.h"
#include "soundmanager.h"

#include "being/localplayer.h"

#include "net/partyhandler.h"
#include "net/serverfeatures.h"

#include "gui/windows/chatwindow.h"

#include "utils/booleanoptions.h"

#include "debug.h"

PartyTab *partyTab = nullptr;

PartyTab::PartyTab(const Widget2 *const widget) :
    // TRANSLATORS: party chat tab name
    ChatTab(widget, _("Party"), "", "#Party", ChatTabType::PARTY)
{
    setTabColors(Theme::PARTY_CHAT_TAB);
    mShowOnline = config.getBoolValue("showPartyOnline");
    config.addListener("showPartyOnline", this);
}

PartyTab::~PartyTab()
{
    config.removeListeners(this);
    CHECKLISTENERS
}

void PartyTab::handleInput(const std::string &msg)
{
    partyHandler->chat(ChatWindow::doReplace(msg));
}

bool PartyTab::handleCommand(const std::string &restrict type,
                             const std::string &restrict args)
{
    if (type == "create" || type == "new")
    {
        if (args.empty())
        {
            // TRANSLATORS: chat error message
            chatLog(_("Party name is missing."), ChatMsgType::BY_SERVER);
        }
        else
        {
            partyHandler->create(args);
        }
    }
    else if (type == "invite")
    {
        partyHandler->invite(args);
    }
    else if (type == "leave")
    {
        partyHandler->leave();
    }
    else if (type == "kick")
    {
        partyHandler->kick(args);
    }
    else if (type == "item")
    {
        if (args.empty())
        {
            switch (partyHandler->getShareItems())
            {
                case Net::PartyShare::YES:
                    // TRANSLATORS: chat message
                    chatLog(_("Item sharing enabled."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::NO:
                    // TRANSLATORS: chat message
                    chatLog(_("Item sharing disabled."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::NOT_POSSIBLE:
                    // TRANSLATORS: chat message
                    chatLog(_("Item sharing not possible."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::UNKNOWN:
                    // TRANSLATORS: chat message
                    chatLog(_("Item sharing unknown."),
                        ChatMsgType::BY_SERVER);
                    return true;
                default:
                    break;
            }
        }

        const signed char opt = parseBoolean(args);

        switch (opt)
        {
            case 1:
                partyHandler->setShareItems(
                    Net::PartyShare::YES);
                break;
            case 0:
                partyHandler->setShareItems(
                    Net::PartyShare::NO);
                break;
            case -1:
                chatLog(strprintf(BOOLEAN_OPTIONS, "item"));
                break;
            default:
                break;
        }
    }
    else if (type == "exp")
    {
        if (args.empty())
        {
            switch (partyHandler->getShareExperience())
            {
                case Net::PartyShare::YES:
                    // TRANSLATORS: chat message
                    chatLog(_("Experience sharing enabled."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::NO:
                    // TRANSLATORS: chat message
                    chatLog(_("Experience sharing disabled."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::NOT_POSSIBLE:
                    // TRANSLATORS: chat message
                    chatLog(_("Experience sharing not possible."),
                        ChatMsgType::BY_SERVER);
                    return true;
                case Net::PartyShare::UNKNOWN:
                    // TRANSLATORS: chat message
                    chatLog(_("Experience sharing unknown."),
                        ChatMsgType::BY_SERVER);
                    return true;
                default:
                    break;
            }
        }

        const signed char opt = parseBoolean(args);

        switch (opt)
        {
            case 1:
                partyHandler->setShareExperience(
                    Net::PartyShare::YES);
                break;
            case 0:
                partyHandler->setShareExperience(
                    Net::PartyShare::NO);
                break;
            case -1:
                chatLog(strprintf(BOOLEAN_OPTIONS, "exp"));
                break;
            default:
                break;
        }
    }
    else if (type == "setleader"
             && serverFeatures->haveChangePartyLeader())
    {
        partyHandler->changeLeader(args);
    }
    else
    {
        return false;
    }

    return true;
}

void PartyTab::getAutoCompleteList(StringVect &names) const
{
    if (!localPlayer)
        return;

    const Party *const p = localPlayer->getParty();

    if (p)
        p->getNames(names);
}

void PartyTab::getAutoCompleteCommands(StringVect &names) const
{
    names.push_back("/help");
    names.push_back("/invite ");
    names.push_back("/leave");
    names.push_back("/kick ");
    names.push_back("/item");
    names.push_back("/exp");
    if (serverFeatures->haveChangePartyLeader())
        names.push_back("/setleader ");
}

void PartyTab::playNewMessageSound() const
{
    soundManager.playGuiSound(SOUND_GUILD);
}

void PartyTab::optionChanged(const std::string &value)
{
    if (value == "showPartyOnline")
        mShowOnline = config.getBoolValue("showPartyOnline");
}
