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

#ifndef NET_TMWA_CHATHANDLER_H
#define NET_TMWA_CHATHANDLER_H

#include "net/ea/chathandler.h"

namespace TmwAthena
{
class MessageOut;

class ChatHandler final : public Ea::ChatHandler
{
    public:
        ChatHandler();

        A_DELETE_COPY(ChatHandler)

        void talk(const std::string &restrict text,
                  const std::string &restrict channel) const override final;

        void talkRaw(const std::string &text) const override final;

        void privateMessage(const std::string &restrict recipient,
                            const std::string &restrict text) override final;

        void channelMessage(const std::string &restrict channel,
                            const std::string &restrict text) override final;

        void joinChannel(const std::string &channel) override final A_CONST;

        void who() const override final;

        void sendRaw(const std::string &args) const override final;

        void ignoreAll() const override final;

        void unIgnoreAll() const override final;

        void createChatRoom(const std::string &title,
                            const std::string &password,
                            const int limit,
                            const bool isPublic) override final A_CONST;

        void ignore(const std::string &nick) const override final;

        void unIgnore(const std::string &nick) const override final;

        void requestIgnoreList() const override final A_CONST;

        void battleTalk(const std::string &text) const override final A_CONST;

        void joinChat(const ChatObject *const chat,
                      const std::string &password) const override final
                      A_CONST;

        void partChannel(const std::string &channel) override final A_CONST;

        void talkPet(const std::string &restrict text,
                     const std::string &restrict channel) const override final;

        void leaveChatRoom() const override final A_CONST;

        void setChatRoomOptions(const int limit,
                                const bool isPublic,
                                const std::string &password,
                                const std::string &title) const override final
                                A_CONST;

        void setChatRoomOwner(const std::string &nick) const override final
                              A_CONST;

        void kickFromChatRoom(const std::string &nick) const override final
                              A_CONST;

    protected:
        static void processRaw(MessageOut &restrict outMsg,
                               const std::string &restrict line);
};

}  // namespace TmwAthena

#endif  // NET_TMWA_CHATHANDLER_H
