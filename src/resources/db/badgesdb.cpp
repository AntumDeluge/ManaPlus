/*
 *  The ManaPlus Client
 *  Copyright (C) 2014-2016  The ManaPlus Developers
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

#include "resources/db/badgesdb.h"

#include "configuration.h"

#include "utils/files.h"
#include "utils/xmlutils.h"

#include "debug.h"

namespace
{
    BadgesInfos mGuilds;
    BadgesInfos mNames;
    BadgesInfos mParties;
    bool mLoaded = false;
}

static void loadXmlFile(const std::string &file,
                        const std::string &name,
                        BadgesInfos &arr)
{
    readXmlStringMap(file,
        "badges",
        name,
        "badge",
        "name",
        "image",
        arr);
}

static void loadDB(const std::string &name, BadgesInfos &arr)
{
    loadXmlFile(paths.getStringValue("badgesFile"),
        name, arr);
    loadXmlFile(paths.getStringValue("badgesPatchFile"),
        name, arr);

    StringVect listVect;
    Files::getFilesInDir(paths.getStringValue(
        "badgesPatchDir"), listVect, ".xml");
    FOR_EACH (StringVectCIter, itVect, listVect)
        loadXmlFile(*itVect, name, arr);
}

void BadgesDB::load()
{
    if (mLoaded)
        unload();

    loadDB("guild", mGuilds);
    loadDB("name", mNames);
    loadDB("party", mParties);
}

void BadgesDB::unload()
{
    mParties.clear();
    mGuilds.clear();
    mNames.clear();
    mLoaded = false;
}

const std::string BadgesDB::getPartyBadge(const std::string &name)
{
    BadgesInfosIter it = mParties.find(name);
    if (it == mParties.end())
        return std::string();
    return (*it).second;
}

const std::string BadgesDB::getNameBadge(const std::string &name)
{
    BadgesInfosIter it = mNames.find(name);
    if (it == mNames.end())
        return std::string();
    return (*it).second;
}

const std::string BadgesDB::getGuildBadge(const std::string &name)
{
    BadgesInfosIter it = mGuilds.find(name);
    if (it == mGuilds.end())
        return std::string();
    return (*it).second;
}
