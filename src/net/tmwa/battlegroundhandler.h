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

#ifndef NET_TMWA_BATTLEGROUNDHANDLER_H
#define NET_TMWA_BATTLEGROUNDHANDLER_H

#ifdef EATHENA_SUPPORT

#include "net/battlegroundhandler.h"

namespace TmwAthena
{

class BattleGroundHandler final : public Net::BattleGroundHandler
{
    public:
        BattleGroundHandler();

        A_DELETE_COPY(BattleGroundHandler)

        void registerBg(const BattleGroundTypeT &type,
                        const std::string &name) const override final A_CONST;

        void rekoveRequest(const std::string &name) const override final
                           A_CONST;

        void beginAck(const bool result,
                      const std::string &bgName,
                      const std::string &gameName) const override final
                      A_CONST;

        void checkState(const std::string &name) const override final A_CONST;
};

}  // namespace TmwAthena

#endif  // EATHENA_SUPPORT
#endif  // NET_TMWA_BATTLEGROUNDHANDLER_H
