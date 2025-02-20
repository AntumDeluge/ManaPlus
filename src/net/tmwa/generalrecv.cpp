/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2019  The ManaPlus Developers
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

#include "net/tmwa/generalrecv.h"

#include "client.h"
#include "logger.h"
#include "settings.h"


#include "net/messagein.h"

#include "utils/cast.h"
#include "utils/gettext.h"

#include "debug.h"

namespace TmwAthena
{

ServerInfo charServer;
ServerInfo mapServer;

void GeneralRecv::processConnectionProblem(Net::MessageIn &msg)
{
    const uint8_t code = msg.readUInt8("flag");
    logger->log("Connection problem: %u", CAST_U32(code));

    switch (code)
    {
        case 0:
            // TRANSLATORS: error message
            errorMessage = _("Authentication failed.");
            break;
        case 1:
            // TRANSLATORS: error message
            errorMessage = _("No servers available.");
            break;
        case 2:
            if (client->getState() == State::GAME)
            {
                // TRANSLATORS: error message
                errorMessage = _("Someone else is trying to use this "
                         "account.");
            }
            else
            {
                // TRANSLATORS: error message
                errorMessage = _("This account is already logged in.");
            }
            break;
        case 3:
            // TRANSLATORS: error message
            errorMessage = _("Speed hack detected.");
            break;
        // all launcher (-S) cases should be added here (exitcode = code)
        case 7:
            if (settings.options.uniqueSession)
            {
                settings.exitcode = code;
                client->setState(State::EXIT);
                return;
            }
            else
            {
                // TRANSLATORS: error message
                errorMessage = _("Wrong Argument set! "
                    "(launcher option)");
            }
            break;
        case 8:
            // TRANSLATORS: error message
            errorMessage = _("Duplicated login.");
            break;
        default:
            // TRANSLATORS: error message
            errorMessage = _("Unknown connection error.");
            break;
    }
    client->setState(State::ERROR);
}

}  // namespace TmwAthena
