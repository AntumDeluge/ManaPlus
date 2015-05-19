/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *
 *
 * Per Larsson a.k.a finalman
 * Olof Naessén a.k.a jansem/yakslem
 *
 * Visit: http://guichan.sourceforge.net
 *
 * License: (BSD)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Guichan nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gui/fonts/font.h"

#include "gui/fonts/textchunk.h"

#include "render/graphics.h"

#include "resources/image.h"
#include "resources/imagehelper.h"

#include "utils/delete2.h"
#include "utils/files.h"
#include "utils/paths.h"
#include "utils/sdlcheckutils.h"
#include "utils/stringutils.h"
#include "utils/timer.h"

#include "debug.h"

const unsigned int CACHE_SIZE = 256;
const unsigned int CACHE_SIZE_SMALL1 = 2;
const unsigned int CACHE_SIZE_SMALL2 = 50;
const unsigned int CACHE_SIZE_SMALL3 = 170;
const unsigned int CLEAN_TIME = 7;

bool Font::mSoftMode(false);

extern char *strBuf;

static int fontCounter;

Font::Font(std::string filename,
           const int size,
           const int style) :
    mFont(nullptr),
    mCreateCounter(0),
    mDeleteCounter(0),
    mCleanTime(cur_time + CLEAN_TIME)
{
    if (fontCounter == 0)
    {
        mSoftMode = imageHelper->useOpenGL() == RENDER_SOFTWARE;
        if (TTF_Init() == -1)
        {
            logger->error("Unable to initialize SDL_ttf: " +
                std::string(TTF_GetError()));
        }
    }

    if (!fontCounter)
    {
        strBuf = new char[65535];
        memset(strBuf, 0, 65535);
    }

    ++fontCounter;

    fixDirSeparators(filename);
    mFont = openFont(filename.c_str(), size);

    if (!mFont)
    {
        logger->log("Error finding font " + filename);
        std::string backFile("fonts/dejavusans.ttf");
        mFont = openFont(fixDirSeparators(backFile).c_str(), size);
        if (!mFont)
        {
            logger->error("Font::Font: " +
                          std::string(TTF_GetError()));
        }
    }

    TTF_SetFontStyle(mFont, style);
}

Font::~Font()
{
    TTF_CloseFont(mFont);
    mFont = nullptr;
    --fontCounter;
    clear();

    if (fontCounter == 0)
    {
        TTF_Quit();
        delete []strBuf;
    }
}

TTF_Font *Font::openFont(const char *const name, const int size)
{
// disabled for now because some systems like gentoo can't use it
// #ifdef USE_SDL2
//    SDL_RWops *const rw = MPHYSFSRWOPS_openRead(name);
//    if (!rw)
//        return nullptr;
//    return TTF_OpenFontIndexRW(rw, 1, size, 0);
// #else
    return TTF_OpenFontIndex(Files::getPath(name).c_str(),
        size, 0);
// #endif
}

void Font::loadFont(std::string filename,
                    const int size,
                    const int style)
{
    if (fontCounter == 0 && TTF_Init() == -1)
    {
        logger->log("Unable to initialize SDL_ttf: " +
                    std::string(TTF_GetError()));
        return;
    }

    fixDirSeparators(filename);
    TTF_Font *const font = openFont(filename.c_str(), size);

    if (!font)
    {
        logger->log("Font::Font: " +
                    std::string(TTF_GetError()));
        return;
    }

    if (mFont)
        TTF_CloseFont(mFont);

    mFont = font;
    TTF_SetFontStyle(mFont, style);
    clear();
}

void Font::clear()
{
    for (size_t f = 0; f < CACHES_NUMBER; f ++)
        mCache[f].clear();
}

void Font::drawString(Graphics *const graphics,
                      const std::string &text,
                      const int x, const int y)
{
    BLOCK_START("Font::drawString")
    if (text.empty())
    {
        BLOCK_END("Font::drawString")
        return;
    }

    Color col = graphics->getColor();
    const Color &col2 = graphics->getColor2();
    const float alpha = static_cast<float>(col.a) / 255.0F;

    /* The alpha value is ignored at string generation so avoid caching the
     * same text with different alpha values.
     */
    col.a = 255;

    const unsigned char chr = text[0];
    TextChunkList *const cache = &mCache[chr];

    std::map<TextChunkSmall, TextChunk*> &search = cache->search;
    std::map<TextChunkSmall, TextChunk*>::iterator i
        = search.find(TextChunkSmall(text, col, col2));
    if (i != search.end())
    {
        TextChunk *const chunk2 = (*i).second;
        cache->moveToFirst(chunk2);
        Image *const image = chunk2->img;
        if (image)
        {
            image->setAlpha(alpha);
            graphics->drawImage(image, x, y);
        }
    }
    else
    {
        if (cache->size >= CACHE_SIZE)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter ++;
#endif
            cache->removeBack();
        }
#ifdef DEBUG_FONT_COUNTERS
        mCreateCounter ++;
#endif
        TextChunk *chunk2 = new TextChunk(text, col, col2, this);

        chunk2->generate(mFont, alpha);
        cache->insertFirst(chunk2);

        const Image *const image = chunk2->img;
        if (image)
            graphics->drawImage(image, x, y);
    }
    BLOCK_END("Font::drawString")
}

void Font::slowLogic(const int rnd)
{
    BLOCK_START("Font::slowLogic")
    if (!mCleanTime)
    {
        mCleanTime = cur_time + CLEAN_TIME + rnd;
    }
    else if (mCleanTime < cur_time)
    {
        doClean();
        mCleanTime = cur_time + CLEAN_TIME + rnd;
    }
    BLOCK_END("Font::slowLogic")
}

int Font::getWidth(const std::string &text) const
{
    if (text.empty())
        return 0;

    const unsigned char chr = text[0];
    TextChunkList *const cache = &mCache[chr];

    std::map<std::string, TextChunk*> &search = cache->searchWidth;
    std::map<std::string, TextChunk*>::iterator i = search.find(text);
    if (i != search.end())
    {
        TextChunk *const chunk = (*i).second;
        cache->moveToFirst(chunk);
        const Image *const image = chunk->img;
        if (image)
            return image->getWidth();
        else
            return 0;
    }

    // if string was not drawed
    int w, h;
    getSafeUtf8String(text, strBuf);
    TTF_SizeUTF8(mFont, strBuf, &w, &h);
    return w;
}

int Font::getHeight() const
{
    return TTF_FontHeight(mFont);
}

void Font::doClean()
{
    for (unsigned int f = 0; f < CACHES_NUMBER; f ++)
    {
        TextChunkList *const cache = &mCache[f];
        const size_t size = static_cast<size_t>(cache->size);
#ifdef DEBUG_FONT_COUNTERS
        logger->log("ptr: %u, size: %ld", f, size);
#endif
        if (size > CACHE_SIZE_SMALL3)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter += 100;
#endif
            cache->removeBack(100);
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete3");
#endif
        }
        else if (size > CACHE_SIZE_SMALL2)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter += 20;
#endif
            cache->removeBack(20);
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete2");
#endif
        }
        else if (size > CACHE_SIZE_SMALL1)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter ++;
#endif
            cache->removeBack();
#ifdef DEBUG_FONT_COUNTERS
            logger->log("delete1");
#endif
        }
    }
}

int Font::getStringIndexAt(const std::string& text, const int x) const
{
    const size_t sz = text.size();
    for (size_t i = 0; i < sz; ++i)
    {
        if (getWidth(text.substr(0, i)) > x)
            return static_cast<int>(i);
    }

    return static_cast<int>(sz);
}

const TextChunkList *Font::getCache() const
{
    return mCache;
}

void Font::generate(TextChunk &chunk)
{
    const std::string &text = chunk.text;
    if (text.empty())
        return;

    const unsigned char chr = text[0];
    TextChunkList *const cache = &mCache[chr];
    Color &col = chunk.color;
    Color &col2 = chunk.color2;
    const int oldAlpha = col.a;
    col.a = 255;

    TextChunkSmall key(text, col, col2);
    std::map<TextChunkSmall, TextChunk*> &search = cache->search;
    std::map<TextChunkSmall, TextChunk*>::iterator i = search.find(key);
    if (i != search.end())
    {
        TextChunk *const chunk2 = (*i).second;
        cache->moveToFirst(chunk2);
        //search.erase(key);
        cache->remove(chunk2);
        chunk.img = chunk2->img;
//        logger->log("cached image: " + chunk.text);
    }
    else
    {
        if (cache->size >= CACHE_SIZE)
        {
#ifdef DEBUG_FONT_COUNTERS
            mDeleteCounter ++;
#endif
            cache->removeBack();
        }
#ifdef DEBUG_FONT_COUNTERS
        mCreateCounter ++;
#endif
        const float alpha = static_cast<float>(chunk.color.a) / 255.0F;
        chunk.generate(mFont, alpha);
//        logger->log("generate image: " + chunk.text);
    }
    col.a = oldAlpha;
}

void Font::insertChunk(TextChunk *const chunk)
{
    if (!chunk || chunk->text.empty() || !chunk->img)
        return;
//    logger->log("insert chunk: text=%s, color: %d,%d,%d",
//        chunk->text.c_str(), chunk->color.r, chunk->color.g, chunk->color.b);
    const unsigned char chr = chunk->text[0];
    TextChunkList *const cache = &mCache[chr];

    std::map<TextChunkSmall, TextChunk*> &search = cache->search;
    std::map<TextChunkSmall, TextChunk*>::iterator i
        = search.find(TextChunkSmall(chunk->text,
        chunk->color, chunk->color2));
    if (i != search.end())
    {
        delete2(chunk->img);
        return;
    }

    TextChunk *const chunk2 = new TextChunk(chunk->text,
        chunk->color, chunk->color2, chunk->textFont);
    chunk2->img = chunk->img;
    cache->insertFirst(chunk2);
}
