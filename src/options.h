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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
#include <cstdint>
#else
#include <stdint.h>
#endif

/**
 * A structure holding the values of various options that can be passed
 * from the command line.
 */
struct Options
{
    Options() :
        username(),
        password(),
        character(),
        brandingPath(),
        updateHost(),
        dataPath(),
        homeDir(),
        logFileName(),
        chatLogDir(),
        configDir(),
        localDataDir(),
        screenshotDir(),
        test(),
        serverName(),
        serverPort(0),
        printHelp(false),
        printVersion(false),
        skipUpdate(false),
        chooseDefault(false),
        noOpenGL(false),
        safeMode(false),
        testMode(false)
    {}

    std::string username;
    std::string password;
    std::string character;
    std::string brandingPath;
    std::string updateHost;
    std::string dataPath;
    std::string homeDir;
    std::string logFileName;
    std::string chatLogDir;
    std::string configDir;
    std::string localDataDir;
    std::string screenshotDir;
    std::string test;
    std::string serverName;
    uint16_t serverPort;
    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    bool noOpenGL;
    bool safeMode;
    bool testMode;
};

#endif  // OPTIONS_H
