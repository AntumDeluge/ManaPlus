/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2017  The ManaPlus Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef GUI_WIDGETS_BROWSERBOX_H
#define GUI_WIDGETS_BROWSERBOX_H

#include "enums/simpletypes/opaque.h"

#include "enums/gui/browserboxmode.h"
#include "enums/gui/colorname.h"

#include "gui/browserlink.h"

#include "gui/widgets/linepart.h"
#include "gui/widgets/widget.h"

#include "listeners/mouselistener.h"
#include "listeners/widgetlistener.h"

#include "localconsts.h"

class LinkHandler;

/**
 * A simple browser box able to handle links and forward events to the
 * parent conteiner.
 */
class BrowserBox final : public Widget,
                         public MouseListener,
                         public WidgetListener
{
    public:
        /**
         * Constructor.
         */
        BrowserBox(const Widget2 *const widget,
                   const BrowserBoxModeT mode,
                   const Opaque opaque,
                   const std::string &skin);

        A_DELETE_COPY(BrowserBox)

        /**
         * Destructor.
         */
        ~BrowserBox();

        /**
         * Sets the handler for links.
         */
        void setLinkHandler(LinkHandler *linkHandler);

        /**
         * Sets the BrowserBox opacity.
         */
        void setOpaque(Opaque opaque)
        { mOpaque = opaque; }

        /**
         * Sets the maximum numbers of rows in the browser box. 0 = no limit.
         */
        void setMaxRow(unsigned max)
        { mMaxRows = max; }

        /**
         * Adds a text row to the browser.
         */
        void addRow(const std::string &row, const bool atTop = false);

        /**
         * Adds a menu line to the browser.
         */
        void addRow(const std::string &cmd, const char *const text);

        void addImage(const std::string &path);

        /**
         * Remove all rows.
         */
        void clearRows();

        /**
         * Handles mouse actions.
         */
        void mousePressed(MouseEvent &event) override final;

        void mouseMoved(MouseEvent &event) override final;

        void mouseExited(MouseEvent& event) override final;

        /**
         * Draws the browser box.
         */
        void draw(Graphics *const graphics) override final A_NONNULL(2);

        void safeDraw(Graphics *const graphics) override final A_NONNULL(2);

        void updateHeight();

        typedef std::list<std::string> TextRows;

        TextRows &getRows() noexcept2 A_WARN_UNUSED
        { return mTextRows; }

        bool hasRows() const noexcept2 A_WARN_UNUSED
        { return !mTextRows.empty(); }

        void setAlwaysUpdate(const bool n) noexcept2
        { mAlwaysUpdate = n; }

        void setProcessVars(const bool n) noexcept2
        { mProcessVars = n; }

        void setEnableImages(const bool n) noexcept2
        { mEnableImages = n; }

        void setEnableKeys(const bool n) noexcept2
        { mEnableKeys = n; }

        void setEnableTabs(const bool n) noexcept2
        { mEnableTabs = n; }

        std::string getTextAtPos(const int x, const int y) const A_WARN_UNUSED;

        int getPadding() const noexcept2 A_WARN_UNUSED
        { return mPadding; }

        void setForegroundColorAll(const Color &color1,
                                   const Color &color2);

        unsigned int getDataWidth() const noexcept2 A_WARN_UNUSED
        { return mDataWidth; }

        void moveSelectionUp();

        void moveSelectionDown();

        void selectSelection();

        void widgetResized(const Event &event) override final;

    private:
        int calcHeight() A_WARN_UNUSED;

        typedef TextRows::iterator TextRowIterator;
        typedef TextRows::const_iterator TextRowCIter;
        TextRows mTextRows;
        std::list<int> mTextRowLinksCount;

        typedef std::vector<LinePart> LinePartList;
        typedef LinePartList::iterator LinePartIterator;
        typedef LinePartList::const_iterator LinePartCIter;
        LinePartList mLineParts;

        typedef std::vector<BrowserLink> Links;
        typedef Links::iterator LinkIterator;
        Links mLinks;

        LinkHandler *mLinkHandler;
        Skin *mSkin;
        BrowserBoxModeT mMode;
        unsigned int mHighlightMode;
        int mSelectedLink;
        unsigned int mMaxRows;
        int mHeight;
        int mWidth;
        int mYStart;
        time_t mUpdateTime;
        int mPadding;
        unsigned int mNewLinePadding;
        int mItemPadding;
        unsigned int mDataWidth;

        Color mHighlightColor;
        Color mHyperLinkColor;
        Color mColors[2][ColorName::COLORS_MAX];

        Opaque mOpaque;
        bool mUseLinksAndUserColors;
        bool mUseEmotes;
        bool mAlwaysUpdate;
        bool mProcessVars;
        bool mEnableImages;
        bool mEnableKeys;
        bool mEnableTabs;

        static ImageSet *mEmotes;
        static int mInstances;
};

#endif  // GUI_WIDGETS_BROWSERBOX_H
