/*
 *  The ManaPlus Client
 *  Copyright (C) 2016-2016  The ManaPlus Developers
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
#include "client.h"
#include "logger.h"

#include "utils/translation/podict.h"
#include "utils/translation/poparser.h"

#include "resources/resourcemanager.h"

#include "utils/physfstools.h"

#include "debug.h"

TEST_CASE("PoParser tests", "PoParser")
{
    client = new Client;
    PHYSFS_init("manaplus");
    dirSeparator = "/";
    logger = new Logger();
    ResourceManager::init();
    resourceManager->addToSearchPath("data", Append_false);
    resourceManager->addToSearchPath("../data", Append_false);

    SECTION("PoParser empty")
    {
        PoParser *parser = new PoParser;
        PoDict *dict = parser->load("ru",
            "unknownfilename.po",
            nullptr);

        REQUIRE(dict != nullptr);
        REQUIRE(dict->getMap() != nullptr);
        REQUIRE(dict->getMap()->size() == 0);

        delete parser;
        delete dict;
    }

    SECTION("PoParser normal")
    {
        PoParser *parser = new PoParser;
        PoDict *dict = parser->load("ru",
            "test/test1",
            nullptr);

        REQUIRE(dict != nullptr);
        REQUIRE(dict->getMap() != nullptr);
        REQUIRE(dict->getMap()->size() == 1787);
        REQUIRE(dict->getStr("Unknown skill message.") ==
            "Неизвестная ошибка скилов.");
        REQUIRE(dict->getStr("Full strip failed because of coating.") ==
            "Full strip failed because of coating.");
        REQUIRE(dict->getStr("You picked up %d [@@%d|%s@@].") ==
            "Вы подняли %d [@@%d|%s@@].");

        delete parser;
        delete dict;
    }
}
