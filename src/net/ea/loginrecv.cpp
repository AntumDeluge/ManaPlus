/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2016  The ManaPlus Developers
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

#include "net/ea/loginrecv.h"

#include "client.h"
#include "configuration.h"

#include "being/being.h"

#include "gui/windows/logindialog.h"

#include "utils/gettext.h"
#include "utils/paths.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/messagein.h"

#include "debug.h"

namespace Ea
{

namespace LoginRecv
{
    std::string mUpdateHost;
    Worlds mWorlds;
    Token mToken;
    bool mVersionResponse = false;
    bool mRegistrationEnabled = true;
}  // namespace LoginRecv

void LoginRecv::processUpdateHost(Net::MessageIn &msg)
{
    const int len = msg.readInt16("len") - 4;
    mUpdateHost = msg.readString(len, "update host");

    if (!checkPath(mUpdateHost))
    {
        mUpdateHost.clear();
        logger->log1("Warning: incorrect update server name");
    }
    loginData.updateHost = mUpdateHost;

    logger->log("Received update host \"%s\" from login server.",
        mUpdateHost.c_str());
}

void LoginRecv::processLoginData(Net::MessageIn &msg)
{
    msg.readInt16("len");

    loginHandler->clearWorlds();

    const int worldCount = (msg.getLength() - 47) / 32;

    mToken.session_ID1 = msg.readInt32("session id1");
    mToken.account_ID = msg.readBeingId("accound id");
    mToken.session_ID2 = msg.readInt32("session id2");
    msg.readInt32("old ip");
    loginData.lastLogin = msg.readString(24, "last login");
    msg.readInt16("unused");

    // reserve bits for future usage
    mToken.sex = Being::intToGender(CAST_U8(
        msg.readUInt8("gender") & 3U));

    for (int i = 0; i < worldCount; i++)
    {
        WorldInfo *const world = new WorldInfo;

        world->address = msg.readInt32("ip address");
        world->port = msg.readInt16("port");
        world->name = msg.readString(20, "name");
        world->online_users = msg.readInt16("online number");
        config.setValue("updatehost", mUpdateHost);
        world->updateHost = mUpdateHost;
        msg.readInt16("maintenance");
        msg.readInt16("new");

        logger->log("Network: Server: %s (%s:%d)", world->name.c_str(),
            ipToString(world->address), world->port);

        mWorlds.push_back(world);
    }
    client->setState(State::WORLD_SELECT);
}

void LoginRecv::processLoginError(Net::MessageIn &msg)
{
    const uint8_t code = msg.readUInt8("error");
    logger->log("Login::error code: %u", CAST_U32(code));
    std::string date = msg.readString(20, "date");

    switch (code)
    {
        case 0:
            // TRANSLATORS: error message
            errorMessage = _("Unregistered ID.");
            break;
        case 1:
            // TRANSLATORS: error message
            errorMessage = _("Wrong password.");
            LoginDialog::savedPassword.clear();
            break;
        case 2:
            // TRANSLATORS: error message
            errorMessage = _("Account expired.");
            break;
        case 3:
            // TRANSLATORS: error message
            errorMessage = _("Rejected from server.");
            break;
        case 4:
            // TRANSLATORS: error message
            errorMessage = _("You have been permanently banned from "
                              "the game. Please contact the GM team.");
            break;
        case 5:
            // TRANSLATORS: error message
            errorMessage = _("Client too old or wrong server type.");
            break;
        case 6:
            // TRANSLATORS: error message
            errorMessage = strprintf(_("You have been temporarily "
                "banned from the game until %s.\nPlease contact the GM "
                "team via the forums."), date.c_str());
            break;
        case 7:
            // TRANSLATORS: error message
            errorMessage = _("Server overpopulated.");
            break;
        case 9:
            // TRANSLATORS: error message
            errorMessage = _("This user name is already taken.");
            break;
        case 10:
            // TRANSLATORS: error message
            errorMessage = _("Wrong name.");
            break;
        case 11:
            // TRANSLATORS: error message
            errorMessage = _("Incorrect email.");
            break;
        case 99:
            // TRANSLATORS: error message
            errorMessage = _("Username permanently erased.");
            break;
        default:
            // TRANSLATORS: error message
            errorMessage = _("Unknown error.");
            break;
    }
    client->setState(State::ERROR);
}

}  // namespace Ea
