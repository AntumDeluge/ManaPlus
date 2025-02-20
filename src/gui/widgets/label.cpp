/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2011-2019  The ManaPlus Developers
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

#include "gui/widgets/label.h"

#include "gui/gui.h"
#include "gui/skin.h"

#include "gui/fonts/font.h"

#include "debug.h"

Skin *Label::mSkin = nullptr;
int Label::mInstances = 0;

Label::Label(const Widget2 *const widget) :
    Widget(widget),
    WidgetListener(),
    ToolTipListener(),
    mCaption(),
    mTextChunk(),
    mAlignment(Graphics::LEFT),
    mPadding(0),
    mTextChanged(true)
{
    init();
}

Label::Label(const Widget2 *const widget,
             const std::string &caption) :
    Widget(widget),
    WidgetListener(),
    ToolTipListener(),
    mCaption(caption),
    mTextChunk(),
    mAlignment(Graphics::LEFT),
    mPadding(0),
    mTextChanged(true)
{
    const Font *const font = getFont();
    setWidth(font->getWidth(caption));
    setHeight(font->getHeight());
    init();
}

Label::~Label()
{
    if (mWindow != nullptr)
        mWindow->removeWidgetListener(this);

    if (gui != nullptr)
        gui->removeDragged(this);

    mInstances --;
    if (mInstances == 0)
    {
        if (theme != nullptr)
            theme->unload(mSkin);
    }
    removeMouseListener(this);
    mTextChunk.deleteImage();
}

void Label::init()
{
    addMouseListener(this);
    mAllowLogic = false;
    mForegroundColor = getThemeColor(ThemeColorId::LABEL, 255U);
    mForegroundColor2 = getThemeColor(ThemeColorId::LABEL_OUTLINE, 255U);
    if (mInstances == 0)
    {
        if (theme != nullptr)
        {
            mSkin = theme->load("label.xml",
                "",
                true,
                Theme::getThemePath());
        }
    }
    mInstances ++;

    if (mSkin != nullptr)
        mPadding = mSkin->getPadding();
    else
        mPadding = 0;
    setSelectable(true);
}

void Label::draw(Graphics *const graphics)
{
    BLOCK_START("Label::draw")
    int textX;
    const Rect &rect = mDimension;
    Font *const font = getFont();

    switch (mAlignment)
    {
        case Graphics::LEFT:
        default:
            textX = mPadding;
            break;
        case Graphics::CENTER:
            textX = (rect.width - font->getWidth(mCaption)) / 2;
            break;
        case Graphics::RIGHT:
            if (rect.width > mPadding)
                textX = rect.width - mPadding - font->getWidth(mCaption);
            else
                textX = 0;
            break;
    }

    if (mTextChanged)
    {
        mTextChunk.textFont = font;
        mTextChunk.deleteImage();
        mTextChunk.text = mCaption;
        mTextChunk.color = mForegroundColor;
        mTextChunk.color2 = mForegroundColor2;
        font->generate(mTextChunk);
        mTextChanged = false;
    }

    const Image *const image = mTextChunk.img;
    if (image != nullptr)
    {
        const int textY = rect.height / 2 - font->getHeight() / 2;
        graphics->drawImage(image, textX, textY);
    }
    BLOCK_END("Label::draw")
}

void Label::safeDraw(Graphics *const graphics)
{
    Label::draw(graphics);
}

void Label::adjustSize()
{
    const Font *const font = getFont();
    const int pad2 = 2 * mPadding;
    setWidth(font->getWidth(mCaption) + pad2);
    setHeight(font->getHeight() + pad2);
}

void Label::setForegroundColor(const Color &color)
{
    if (mForegroundColor != color || mForegroundColor2 != color)
        mTextChanged = true;
//    logger->log("Label::setForegroundColor: " + mCaption);
    mForegroundColor = color;
    mForegroundColor2 = color;
}

void Label::setForegroundColorAll(const Color &color1,
                                  const Color &color2)
{
    if (mForegroundColor != color1 || mForegroundColor2 != color2)
        mTextChanged = true;
//    logger->log("Label::setForegroundColorAll: " + mCaption);
    mForegroundColor = color1;
    mForegroundColor2 = color2;
}

void Label::resizeTo(const int maxSize, const int minSize)
{
    const Font *const font = getFont();
    const int pad2 = 2 * mPadding;
    setHeight(font->getHeight() + pad2);

    if (font->getWidth(mCaption) + pad2 > maxSize)
    {
        const int dots = font->getWidth("...");
        if (dots > maxSize)
        {
            setWidth(maxSize);
            return;
        }
        const size_t szChars = mCaption.size();
        for (size_t f = 1; f < szChars - 1; f ++)
        {
            const std::string text = mCaption.substr(0, szChars - f);
            const int width = font->getWidth(text) + dots + pad2;
            if (width <= maxSize)
            {
                setCaption(text + "...");
                setWidth(width);
                return;
            }
        }
        setWidth(maxSize);
    }
    else
    {
        int sz = font->getWidth(mCaption) + pad2;
        if (sz < minSize)
            sz = minSize;
        setWidth(sz);
    }
}

void Label::setCaption(const std::string& caption)
{
    if (caption != mCaption)
        mTextChanged = true;
    mCaption = caption;
}

void Label::setParent(Widget *const widget)
{
    if (mWindow != nullptr)
        mWindow->addWidgetListener(this);
    Widget::setParent(widget);
}

void Label::setWindow(Widget *const widget)
{
    if ((widget == nullptr) && (mWindow != nullptr))
    {
        mWindow->removeWidgetListener(this);
        mWindow = nullptr;
    }
    else
    {
        Widget2::setWindow(widget);
    }
}

void Label::widgetHidden(const Event &event A_UNUSED)
{
    mTextChanged = true;
    mTextChunk.deleteImage();
}

void Label::finalCleanup()
{
    mSkin = nullptr;
}
