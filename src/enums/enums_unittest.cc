/*
 *  The ManaPlus Client
 *  Copyright (C) 2015-2016  The ManaPlus Developers
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

#include "catch.hpp"

#include "enums/being/gender.h"

#include "enums/input/inputaction.h"

#include "debug.h"

TEST_CASE("Gender", "Enums")
{
    REQUIRE(CAST_S32(Gender::FEMALE) == 1);
    REQUIRE(Gender::FEMALE == static_cast<GenderT>(1));
    REQUIRE(CAST_S32(Gender::MALE) == 0);
    REQUIRE(Gender::MALE == static_cast<GenderT>(0));
}
