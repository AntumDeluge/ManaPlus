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

#ifndef NET_EA_CHATRECV_H
#define NET_EA_CHATRECV_H

#include "net/chathandler.h"

#include <queue>

namespace Net
{
    class MessageIn;
}

namespace Ea
{
    typedef std::queue<std::string> WhisperQueue;

    namespace ChatRecv
    {
        void processMVPEffect(Net::MessageIn &msg);
        void processIgnoreAllResponse(Net::MessageIn &msg);
        void processWhisperResponseContinue(Net::MessageIn &msg,
                                            const uint8_t type);
        std::string getPopLastWhisperNick();
        std::string getLastWhisperNick();

        extern WhisperQueue mSentWhispers;
        extern int mMotdTime;
        extern bool mShowAllLang;
        extern bool mShowMotd;
        extern bool mSkipping;
    }  // namespace ChatRecv
}  // namespace Ea

#endif  // NET_EA_CHATRECV_H
