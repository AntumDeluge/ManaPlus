/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#ifndef BEING_PICKUP_H
#define BEING_PICKUP_H

#include "localconsts.h"

namespace Pickup
{
    /**
     * Reasons an item can fail to be picked up.
     */
    enum Type
    {
        OKAY = 0,
        BAD_ITEM,
        TOO_HEAVY,
        TOO_FAR,
        INV_FULL,
        STACK_FULL,
        DROP_STEAL
    };
}  // namespace Pickup

#endif  // BEING_PICKUP_H
