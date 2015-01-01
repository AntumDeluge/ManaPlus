/*
 *  The ManaPlus Client
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

#ifndef NET_MAILHANDLER_H
#define NET_MAILHANDLER_H

#include <string>

#include "localconsts.h"

namespace Net
{

class MailHandler notfinal
{
    public:
        virtual ~MailHandler()
        { }

        virtual void refresh() = 0;

        virtual void readMessage(const int msgId) = 0;

        virtual void getAttach(const int msgId) = 0;

        virtual void deleteMessage(const int msgId) = 0;

        virtual void returnMessage(const int msgId) = 0;

        virtual void setAttach(const int index, const int amount) = 0;

        virtual void resetAttach(const int flag) = 0;

        virtual void send(const std::string &name,
                          const std::string &title,
                          std::string message) = 0;
};

}  // namespace Net

extern Net::MailHandler *mailHandler;

#endif  // NET_MAILHANDLER_H
