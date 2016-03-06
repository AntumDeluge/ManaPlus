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

#ifndef NET_EATHENA_SERVERFEATURES_H
#define NET_EATHENA_SERVERFEATURES_H

#include "net/serverfeatures.h"

#include "localconsts.h"

namespace EAthena
{
class ServerFeatures final : public Net::ServerFeatures
{
    public:
        ServerFeatures();

        A_DELETE_COPY(ServerFeatures)

        bool haveServerOnlineList() const override final;

        bool haveOnlineList() const override final A_CONST;

        bool havePartyNickInvite() const override final A_CONST;

        bool haveChangePartyLeader() const override final A_CONST;

        bool haveServerHp() const override final A_CONST;

        bool havePlayerStatusUpdate() const override final;

        bool haveBrokenPlayerAttackDistance() const override final A_CONST;

        bool haveNativeGuilds() const override final A_CONST;

        bool haveIncompleteChatMessages() const override final A_CONST;

        bool haveRaceSelection() const override final;

        bool haveLookSelection() const override final;

        bool haveChatChannels() const override final A_CONST;

        bool haveServerIgnore() const override final A_CONST;

        bool haveMove3() const override final;

        bool haveItemColors() const override final A_CONST;

        bool haveAccountOtherGender() const override final A_CONST;

        bool haveCharOtherGender() const override final;

        bool haveMonsterAttackRange() const override final A_CONST;

        bool haveMonsterName() const override final A_CONST;

        bool haveEmailOnRegister() const override final;

        bool haveEmailOnDelete() const override final;

        bool haveEightDirections() const override final A_CONST;

        bool haveCharRename() const override final A_CONST;

        bool haveBankApi() const override final A_CONST;

        bool haveServerVersion() const override final;

        bool haveMapServerVersion() const override final;

        bool haveNpcGender() const override final;

        bool haveJoinChannel() const override final;

        bool haveNpcWhispers() const override final A_CONST;

        bool haveCreateCharGender() const override final;

        bool haveAttackDirections() const override final A_CONST;

        bool haveVending() const override final A_CONST;

        bool haveCart() const override final A_CONST;

        bool haveTalkPet() const override final;

        bool haveMovePet() const override final;

        bool haveServerWarpNames() const override final;

        bool haveExpPacket() const override final A_CONST;

        bool haveMute() const override final A_CONST;

        bool haveChangePassword() const override final;

        bool haveTeamId() const override final A_CONST;

        bool haveNewGuild() const override final A_CONST;

        bool haveAdvancedSprites() const override final;

        bool haveExtendedRiding() const override final;

        bool haveAdvancedBuySell() const override final A_CONST;

        bool haveSlide() const override final;

        bool haveExtendedDropsPosition() const override final;

        bool haveSecureTrades() const override final;

        bool haveMultyStatusUp() const override final;

        bool haveMail() const override final;
};

}  // namespace EAthena

#endif  // NET_EATHENA_SERVERFEATURES_H
