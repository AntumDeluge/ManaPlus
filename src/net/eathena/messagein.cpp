/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "net/eathena/messagein.h"

#include "net/net.h"
#include "net/packetcounters.h"

#include "logger.h"

#include <SDL_endian.h>

#include "debug.h"

#ifndef SDL_BYTEORDER
#error missing SDL_endian.h
#endif

namespace EAthena
{

MessageIn::MessageIn(const char *const data,
                     const unsigned int length) :
    Net::MessageIn(data, length)
{
}

void MessageIn::postInit(const char *const str,
                         const unsigned int version)
{
    // Read the message ID
    mId = readId();
    mVersion = version;
    IGNOREDEBUGLOG;
    DEBUGLOG2("Receive packet", 0, "MessageIn");
#ifdef ENABLEDEBUGLOG
    if (mVersion > 0)
    {
        const std::string verStr = toString(mVersion);
        DEBUGLOG2("Version", 0, verStr.c_str());
    }
#endif
    readInt16(str);
}

uint16_t MessageIn::readId() const
{
    int16_t value = -1;
    if (mPos + 2 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int16_t swap;
        memcpy(&swap, mData + CAST_SIZE(mPos), sizeof(int16_t));
        value = SDL_Swap16(swap);
#else
        memcpy(&value, mData + CAST_SIZE(mPos), sizeof(int16_t));
#endif
    }
    return value;
}

int16_t MessageIn::readInt16(const char *const str)
{
    int16_t value = -1;
    if (mPos + 2 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int16_t swap;
        memcpy(&swap, mData + CAST_SIZE(mPos), sizeof(int16_t));
        value = SDL_Swap16(swap);
#else
        memcpy(&value, mData + CAST_SIZE(mPos), sizeof(int16_t));
#endif
    }
    DEBUGLOG2("readInt16:  " + toStringPrint(CAST_U32(
        CAST_U16(value))),
        mPos, str);
    mPos += 2;
    PacketCounters::incInBytes(2);
    return value;
}

uint16_t MessageIn::readUInt16(const char *const str)
{
    uint16_t value = 0xffU;
    if (mPos + 2 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        uint16_t swap;
        memcpy(&swap, mData + CAST_SIZE(mPos), sizeof(uint16_t));
        value = SDL_Swap16(swap);
#else
        memcpy(&value, mData + CAST_SIZE(mPos), sizeof(uint16_t));
#endif
    }
    DEBUGLOG2("readUInt16:  " + toStringPrint(CAST_U32(
        CAST_U16(value))),
        mPos, str);
    mPos += 2;
    PacketCounters::incInBytes(2);
    return value;
}

int32_t MessageIn::readInt32(const char *const str)
{
    int32_t value = -1;
    if (mPos + 4 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int32_t swap;
        memcpy(&swap, mData + CAST_SIZE(mPos), sizeof(int32_t));
        value = SDL_Swap32(swap);
#else
        memcpy(&value, mData + CAST_SIZE(mPos), sizeof(int32_t));
#endif
    }
    DEBUGLOG2("readInt32:  " + toStringPrint(CAST_U32(value)),
        mPos, str);
    mPos += 4;
    PacketCounters::incInBytes(4);
    return value;
}

BeingId MessageIn::readBeingId(const char *const str)
{
    return fromInt(readInt32(str), BeingId);
}

int64_t MessageIn::readInt64(const char *const str)
{
    int64_t value = -1;
    if (mPos + 8 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int64_t swap;
        memcpy(&swap, mData + CAST_SIZE(mPos), sizeof(int64_t));
        value = SDL_Swap64(swap);
#else
        memcpy(&value, mData + CAST_SIZE(mPos), sizeof(int64_t));
#endif
    }
    DEBUGLOG2("readInt64:  " + toStringPrint(CAST_U32(value)),
        mPos, str);
    mPos += 8;
    PacketCounters::incInBytes(8);
    return value;
}

}  // namespace EAthena
