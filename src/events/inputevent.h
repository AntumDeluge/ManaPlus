/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2014  The ManaPlus Developers
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

#ifndef EVENTS_INPUTEVENT_H
#define EVENTS_INPUTEVENT_H

#include <map>
#include <string>
#include <vector>

#include "localconsts.h"

class ChatTab;

typedef std::vector<int> KeysVector;
typedef KeysVector::iterator KeysVectorIter;
typedef KeysVector::const_iterator KeysVectorCIter;

typedef std::map<int, KeysVector> KeyToActionMap;
typedef KeyToActionMap::iterator KeyToActionMapIter;

typedef std::map<int, int> KeyToIdMap;
typedef KeyToIdMap::iterator KeyToIdMapIter;

typedef std::map<int, int> KeyTimeMap;
typedef KeyTimeMap::iterator KeyTimeMapIter;

struct InputEvent final
{
    InputEvent(const int action0, const int mask0) :
        args(),
        tab(nullptr),
        action(action0),
        mask(mask0)
    { }

    InputEvent(const std::string &args0,
               ChatTab *const tab0,
               const int mask0) :
        args(args0),
        tab(tab0),
        action(-1),
        mask(mask0)
    { }

    const std::string args;
    ChatTab *const tab;
    const int action;
    const int mask;
};

#endif  // EVENTS_INPUTEVENT_H
