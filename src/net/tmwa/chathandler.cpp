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

#include "net/tmwa/chathandler.h"

#include "being/localplayer.h"

#include "gui/chatconsts.h"

#include "net/serverfeatures.h"

#include "net/ea/chatrecv.h"

#include "net/tmwa/messageout.h"
#include "net/tmwa/protocol.h"

#include "utils/stringutils.h"

#include "debug.h"

extern Net::ChatHandler *chatHandler;

namespace TmwAthena
{

ChatHandler::ChatHandler() :
    Ea::ChatHandler()
{
    chatHandler = this;
}

void ChatHandler::talk(const std::string &restrict text,
                       const std::string &restrict channel A_UNUSED) const
{
    if (!localPlayer)
        return;

    const std::string mes = std::string(localPlayer->getName()).append(
        " : ").append(text);

    createOutPacket(CMSG_CHAT_MESSAGE);
    // Added + 1 in order to let eAthena parse admin commands correctly
    outMsg.writeInt16(static_cast<int16_t>(mes.length() + 4 + 1), "len");
    outMsg.writeString(mes, static_cast<int>(mes.length() + 1), "message");
}

void ChatHandler::talkRaw(const std::string &mes) const
{
    createOutPacket(CMSG_CHAT_MESSAGE);
    outMsg.writeInt16(static_cast<int16_t>(mes.length() + 4), "len");
    outMsg.writeString(mes, static_cast<int>(mes.length()), "message");
}

void ChatHandler::privateMessage(const std::string &restrict recipient,
                                 const std::string &restrict text)
{
    createOutPacket(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(static_cast<int16_t>(text.length() + 28), "len");
    outMsg.writeString(recipient, 24, "recipient nick");
    outMsg.writeString(text, static_cast<int>(text.length()), "message");
    Ea::ChatRecv::mSentWhispers.push(recipient);
}

void ChatHandler::channelMessage(const std::string &restrict channel,
                                 const std::string &restrict text)
{
    if (channel == TRADE_CHANNEL)
        talk("\302\202" + text, GENERAL_CHANNEL);
    else if (channel == GM_CHANNEL)
        talk("@wgm " + text, GENERAL_CHANNEL);
}

void ChatHandler::who() const
{
    createOutPacket(CMSG_WHO_REQUEST);
}

void ChatHandler::sendRaw(const std::string &args) const
{
    std::string line = args;
    std::string str;
    MessageOut *outMsg = nullptr;

    if (line == "")
        return;

    size_t pos = line.find(" ");
    if (pos != std::string::npos)
    {
        str = line.substr(0, pos);
        outMsg = new MessageOut(static_cast<int16_t>(parseNumber(str)));
        line = line.substr(pos + 1);
        pos = line.find(" ");
    }
    else
    {
        outMsg = new MessageOut(static_cast<int16_t>(parseNumber(line)));
        delete outMsg;
        return;
    }

    while (pos != std::string::npos)
    {
        str = line.substr(0, pos);
        processRaw(*outMsg, str);
        line = line.substr(pos + 1);
        pos = line.find(" ");
    }
    if (line != "")
        processRaw(*outMsg, line);
    delete outMsg;
}

void ChatHandler::processRaw(MessageOut &restrict outMsg,
                             const std::string &restrict line)
{
    if (line.size() < 2)
        return;

    const uint32_t i = parseNumber(line.substr(1));
    switch (tolower(line[0]))
    {
        case 'b':
        {
            outMsg.writeInt8(static_cast<unsigned char>(i), "raw");
            break;
        }
        case 'w':
        {
            outMsg.writeInt16(static_cast<int16_t>(i), "raw");
            break;
        }
        case 'l':
        {
            outMsg.writeInt32(static_cast<int32_t>(i), "raw");
            break;
        }
        default:
            break;
    }
}

void ChatHandler::ignoreAll() const
{
    if (!serverFeatures->haveServerIgnore())
        return;
    createOutPacket(CMSG_IGNORE_ALL);
    outMsg.writeInt8(0, "flag");
}

void ChatHandler::unIgnoreAll() const
{
    if (!serverFeatures->haveServerIgnore())
        return;
    createOutPacket(CMSG_IGNORE_ALL);
    outMsg.writeInt8(1, "flag");
}

void ChatHandler::ignore(const std::string &nick) const
{
    createOutPacket(CMSG_IGNORE_NICK);
    outMsg.writeString(nick, 24, "nick");
    outMsg.writeInt8(0, "flag");
}

void ChatHandler::unIgnore(const std::string &nick) const
{
    createOutPacket(CMSG_IGNORE_NICK);
    outMsg.writeString(nick, 24, "nick");
    outMsg.writeInt8(1, "flag");
}

void ChatHandler::requestIgnoreList() const
{
}

void ChatHandler::createChatRoom(const std::string &title A_UNUSED,
                                 const std::string &password A_UNUSED,
                                 const int limit A_UNUSED,
                                 const bool isPublic A_UNUSED)
{
}

void ChatHandler::battleTalk(const std::string &text A_UNUSED) const
{
}

void ChatHandler::joinChat(const ChatObject *const chat A_UNUSED,
                           const std::string &password A_UNUSED) const
{
}

void ChatHandler::joinChannel(const std::string &channel A_UNUSED)
{
}

void ChatHandler::partChannel(const std::string &channel A_UNUSED)
{
}

void ChatHandler::talkPet(const std::string &restrict text,
                          const std::string &restrict channel) const
{
    // here need string duplication
    std::string action = strprintf("\302\202\303 %s", text.c_str());
    talk(action, channel);
}

void ChatHandler::leaveChatRoom() const
{
}

void ChatHandler::setChatRoomOptions(const int limit A_UNUSED,
                                     const bool isPublict A_UNUSED,
                                     const std::string &passwordt A_UNUSED,
                                     const std::string &titlet A_UNUSED) const
{
}

void ChatHandler::setChatRoomOwner(const std::string &nick A_UNUSED) const
{
}

void ChatHandler::kickFromChatRoom(const std::string &nick A_UNUSED) const
{
}

}  // namespace TmwAthena
