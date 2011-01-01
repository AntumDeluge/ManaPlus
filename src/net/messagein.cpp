/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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

#include "net/messagein.h"

#include "net/packetcounters.h"

#include "log.h"

#include "utils/stringutils.h"

#define MAKEWORD(low, high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))

namespace Net
{

MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mPos(0)
{
    PacketCounters::incInPackets();
    DEBUGLOG("MessageIn");
}

unsigned char MessageIn::readInt8()
{
    unsigned char value = -1;
    if (mPos < mLength)
        value = static_cast<unsigned char>(mData[mPos]);

    mPos += 1;
    PacketCounters::incInBytes(1);
    DEBUGLOG("readInt8: " + toString(static_cast<int>(value)));
    return value;
}

void MessageIn::readCoordinates(Uint16 &x, Uint16 &y)
{
    if (mPos + 3 <= mLength)
    {
        unsigned char const *p
            = reinterpret_cast< unsigned char const * >(mData + mPos);
        x = static_cast<short unsigned>(p[0] | ((p[1] & 0x07) << 8));
        y = static_cast<short unsigned>((p[1] >> 3) | ((p[2] & 0x3F) << 5));
    }
    mPos += 3;
    PacketCounters::incInBytes(3);
    DEBUGLOG("readCoordinates: " + toString(static_cast<int>(x)) + ","
             + toString(static_cast<int>(y)));
}

void MessageIn::readCoordinates(Uint16 &x, Uint16 &y, Uint8 &direction)
{
    if (mPos + 3 <= mLength)
    {
        const char *data = mData + mPos;
        Sint16 temp;

        temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
        x = static_cast<unsigned short>(temp >> 6);
        temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
        y = static_cast<unsigned short>(temp >> 4);

        direction = data[2] & 0x000f;

        // Translate from eAthena format
        switch (direction)
        {
            case 0:
                direction = 1;
                break;
            case 1:
                direction = 3;
                break;
            case 2:
                direction = 2;
                break;
            case 3:
                direction = 6;
                break;
            case 4:
                direction = 4;
                break;
            case 5:
                direction = 12;
                break;
            case 6:
                direction = 8;
                break;
            case 7:
                direction = 9;
                break;
            default:
                // OOPSIE! Impossible or unknown
                direction = 0;
        }
    }
    mPos += 3;
    PacketCounters::incInBytes(3);
    DEBUGLOG("readCoordinates: " + toString((int)x) + "," + toString((int)y));
}

void MessageIn::readCoordinatePair(Uint16 &srcX, Uint16 &srcY,
                                   Uint16 &dstX, Uint16 &dstY)
{
    if (mPos + 5 <= mLength)
    {
        const char *data = mData + mPos;
        Sint16 temp;

        temp = MAKEWORD(data[3], data[2] & 0x000f);
        dstX = static_cast<unsigned short>(temp >> 2);

        dstY = MAKEWORD(data[4], data[3] & 0x0003);

        temp = MAKEWORD(data[1], data[0]);
        srcX = static_cast<unsigned short>(temp >> 6);

        temp = MAKEWORD(data[2], data[1] & 0x003f);
        srcY = static_cast<unsigned short>(temp >> 4);
    }
    mPos += 5;
    DEBUGLOG("readCoordinatePair: " + toString((int)srcX) + ","
             + toString((int)srcY) + " " + toString((int)dstX) + ","
             + toString((int)dstY));
    PacketCounters::incInBytes(5);
}

void MessageIn::skip(unsigned int length)
{
    mPos += length;
    PacketCounters::incInBytes(length);
    DEBUGLOG("skip: " + toString((int)length));
}

std::string MessageIn::readString(int length)
{
    // Get string length
    if (length < 0)
        length = readInt16();

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        DEBUGLOG("readString error");
        return "";
    }

    // Read the string
    char const *stringBeg = mData + mPos;
    char const *stringEnd
        = static_cast<char const *>(memchr(stringBeg, '\0', length));

    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;
    PacketCounters::incInBytes(length);
    DEBUGLOG("readString: " + readString);
    return readString;
}

std::string MessageIn::readRawString(int length)
{
    // Get string length
    if (length < 0)
        length = readInt16();

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return "";
    }

    // Read the string
    char const *stringBeg = mData + mPos;
    char const *stringEnd
        = static_cast<char const *>(memchr(stringBeg, '\0', length));
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);

    mPos += length;
    PacketCounters::incInBytes(length);
    DEBUGLOG("readString: " + readString);

    if (stringEnd)
    {
        long len2 = length - (stringEnd - stringBeg) - 1;
        char const *stringBeg2 = stringEnd + 1;
        char const *stringEnd2
            = static_cast<char const *>(memchr(stringBeg2, '\0', len2));
        std::string hiddenPart = std::string(stringBeg2,
                                stringEnd2 ? stringEnd2 - stringBeg2 : len2);
        if (hiddenPart.length() > 0)
        {
            DEBUGLOG("readString2: " + hiddenPart);

            return readString + "|" + hiddenPart;
        }
    }

    return readString;
}

}
