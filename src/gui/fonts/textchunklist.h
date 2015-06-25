/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2015  The ManaPlus Developers
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

#ifndef GUI_FONTS_TEXTCHUNKLIST_H
#define GUI_FONTS_TEXTCHUNKLIST_H

#include "gui/fonts/textchunksmall.h"

#include <map>

#include "localconsts.h"

class TextChunk;

class TextChunkList final
{
    public:
        TextChunkList();

        A_DELETE_COPY(TextChunkList)

        void insertFirst(TextChunk *const item) A_NONNULL(2);

        void moveToFirst(TextChunk *const item) A_NONNULL(2);

        void remove(const TextChunk *const item);

        void removeBack();

        void removeBack(int n);

        void clear();

        TextChunk *start;
        TextChunk *end;
        uint32_t size;
        std::map<TextChunkSmall, TextChunk*> search;
        std::map<std::string, TextChunk*> searchWidth;
};

#endif  // GUI_FONTS_TEXTCHUNKLIST_H
