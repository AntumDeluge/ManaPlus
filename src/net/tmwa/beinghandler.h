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

#ifndef NET_TMWA_BEINGHANDLER_H
#define NET_TMWA_BEINGHANDLER_H

#include "net/ea/beinghandler.h"

namespace TmwAthena
{

class BeingHandler final : public Ea::BeingHandler
{
    public:
        explicit BeingHandler(const bool enableSync);

        A_DELETE_COPY(BeingHandler)

        void requestNameById(const BeingId id) const override final;

        void undress(Being *const being) const override final;

#ifdef EATHENA_SUPPORT
        void requestRanks(const RankT rank A_UNUSED) const override final;
#endif

    protected:
        void viewPlayerEquipment(const Being *const being);
};

}  // namespace TmwAthena

#endif  // NET_TMWA_BEINGHANDLER_H
