/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2015  The ManaPlus Developers
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

#include "notifymanager.h"

#include "soundmanager.h"

#include "being/localplayer.h"

#include "enums/resources/notifytypes.h"

#include "net/guildhandler.h"
#include "net/partyhandler.h"

#ifdef TMWA_SUPPORT
#include "net/tmwa/guildmanager.h"
#endif

#include "resources/notifications.h"

#include "resources/db/sounddb.h"

#include "utils/stringutils.h"

#include "debug.h"

namespace NotifyManager
{
    static ChatTab *getGuildTab()
    {
        const Guild *const guild = localPlayer->getGuild();
        if (guild)
        {
#ifdef TMWA_SUPPORT
            if (guild->getServerGuild())
                return guildHandler->getTab();
            else if (guildManager)
                return guildManager->getTab();
#else
            return guildHandler->getTab();
#endif
        }
        return nullptr;
    }

    static void chatLog(ChatTab *const tab, const std::string &str)
    {
        if (str.empty())
            return;
        if (tab)
            tab->chatLog(str, ChatMsgType::BY_SERVER);
        else if (debugChatTab)
            debugChatTab->chatLog(str, ChatMsgType::BY_SERVER);
    }

    void notify(const unsigned int message)
    {
        if (message >= NotifyTypes::TYPE_END || !localChatTab)
            return;
        const NotificationInfo &info = notifications[message];
        if (!*info.text)
        {
            soundManager.playSfx(SoundDB::getSound(message));
            return;
        }

        switch (info.flags)
        {
            case NotifyFlags::EMPTY:
                localChatTab->chatLog(gettext(info.text),
                    ChatMsgType::BY_SERVER);
                break;

            case NotifyFlags::GUILD:
            {
                if (!localPlayer)
                    return;
                ChatTab *const tab = getGuildTab();
                chatLog(tab, gettext(info.text));
                break;
            }

            case NotifyFlags::PARTY:
            {
                ChatTab *const tab = partyHandler->getTab();
                chatLog(tab, gettext(info.text));
                break;
            }

            case NotifyFlags::SPEECH:
            {
                if (localPlayer)
                    localPlayer->setSpeech(gettext(info.text));
                break;
            }

            case NotifyFlags::INT:
            case NotifyFlags::STRING:
            case NotifyFlags::GUILD_STRING:
            case NotifyFlags::PARTY_STRING:
            default:
                break;
        }
        soundManager.playSfx(SoundDB::getSound(message));
    }

    void notify(const unsigned int message, const int num)
    {
        if (message >= NotifyTypes::TYPE_END || !localChatTab)
            return;
        const NotificationInfo &info = notifications[message];
        if (info.flags == NotifyFlags::INT && *info.text)
        {
            localChatTab->chatLog(strprintf(gettext(info.text),
                num), ChatMsgType::BY_SERVER);
        }
        soundManager.playSfx(SoundDB::getSound(message));
    }

    void notify(const unsigned int message, const std::string &str)
    {
        if (message >= NotifyTypes::TYPE_END || !localChatTab)
            return;
        const NotificationInfo &info = notifications[message];
        if (!*info.text)
        {
            soundManager.playSfx(SoundDB::getSound(message));
            return;
        }
        switch (info.flags)
        {
            case NotifyFlags::STRING:
            {
                localChatTab->chatLog(strprintf(gettext(info.text),
                    str.c_str()), ChatMsgType::BY_SERVER);
                break;
            }
            case NotifyFlags::GUILD_STRING:
            {
                ChatTab *const tab = getGuildTab();
                chatLog(tab, strprintf(gettext(info.text), str.c_str()));
                break;
            }
            case NotifyFlags::PARTY_STRING:
            {
                ChatTab *const tab = partyHandler->getTab();
                chatLog(tab, strprintf(gettext(info.text), str.c_str()));
                break;
            }
            case NotifyFlags::EMPTY:
            case NotifyFlags::INT:
            case NotifyFlags::GUILD:
            case NotifyFlags::PARTY:
            case NotifyFlags::SPEECH:
            default:
                break;
        }
        soundManager.playSfx(SoundDB::getSound(message));
    }

    int getIndexBySound(const std::string &sound)
    {
        for (int f = 0; f < NotifyTypes::TYPE_END; f ++)
        {
            if (notifications[f].sound == sound)
                return f;
        }
        return 0;
    }
}  // namespace NotifyManager
