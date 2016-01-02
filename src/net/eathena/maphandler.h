/*
 *  The ManaPlus Client
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

#ifndef NET_EATHENA_MAPHANDLER_H
#define NET_EATHENA_MAPHANDLER_H

#include "net/maphandler.h"

namespace Net
{
    class MessageIn;
}

namespace EAthena
{

class MapHandler final : public Net::MapHandler
{
    public:
        MapHandler();

        A_DELETE_COPY(MapHandler)

    protected:
        void processInstanceStart(Net::MessageIn &msg);

        void processInstanceCreate(Net::MessageIn &msg);

        void processInstanceInfo(Net::MessageIn &msg);

        void processInstanceDelete(Net::MessageIn &msg);
};

}  // namespace EAthena

#endif  // NET_EATHENA_MAPHANDLER_H
