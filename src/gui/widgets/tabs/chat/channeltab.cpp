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

#include "gui/widgets/tabs/chat/channeltab.h"

#include "chatlogger.h"
#include "configuration.h"

#include "gui/windows/chatwindow.h"

#include "net/chathandler.h"

#include "debug.h"

ChannelTab::ChannelTab(const Widget2 *const widget,
                       const std::string &channel) :
    ChatTab(widget, channel, channel, channel, ChatTabType::CHANNEL)
{
    setTabColors(Theme::CHANNEL_CHAT_TAB);
}

ChannelTab::~ChannelTab()
{
}

void ChannelTab::handleInput(const std::string &msg)
{
    std::string newMsg;
    newMsg = ChatWindow::doReplace(msg);
    chatHandler->privateMessage(mChannelName, newMsg);
}
