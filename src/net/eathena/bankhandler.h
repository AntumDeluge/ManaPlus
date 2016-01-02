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

#ifndef NET_EATHENA_BANKHANDLER_H
#define NET_EATHENA_BANKHANDLER_H

#ifdef EATHENA_SUPPORT

#include "net/bankhandler.h"

namespace EAthena
{
class BankHandler final : public Net::BankHandler
{
    public:
        BankHandler();

        A_DELETE_COPY(BankHandler)

        void deposit(const int money) const override final;

        void withdraw(const int money) const override final;

        void check() const override final;

        void open() const override final;

        void close() const override final;
};

}  // namespace EAthena

#endif  // EATHENA_SUPPORT
#endif  // NET_EATHENA_BANKHANDLER_H
