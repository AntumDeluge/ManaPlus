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

#ifndef GUI_WIDGETS_CHATINPUT_H
#define GUI_WIDGETS_CHATINPUT_H

#include "gui/windows/chatwindow.h"

#include "configuration.h"

#include "gui/windows/emotewindow.h"

#include "gui/widgets/textfield.h"

#include "localconsts.h"

/**
 * The chat input hides when it loses focus. It is also invisible by default.
 */
class ChatInput final : public TextField
{
    public:
        explicit ChatInput(ChatWindow *const window) :
            TextField(window, "", false),
            mWindow(window),
            mFocusGaining(false)
        {
            setVisible(false);
            addFocusListener(this);
        }

        A_DELETE_COPY(ChatInput)

        /**
         * Called if the chat input loses focus. It will set itself to
         * invisible as result.
         */
        void focusLost(const Event &event)
        {
            TextField::focusLost(event);
            if (mFocusGaining || !config.getBoolValue("protectChatFocus"))
            {
                processVisible(false);
                if (chatWindow)
                    chatWindow->updateVisibility();
                mFocusGaining = false;
                return;
            }
            mFocusGaining = true;
            requestFocus();
            mFocusGaining = false;
        }

        void processVisible(const bool n)
        {
            if (!mWindow || isVisible() == n)
                return;

            if (!n)
                mFocusGaining = true;
            setVisible(n);
            if (config.getBoolValue("hideChatInput")
                || config.getBoolValue("showEmotesButton"))
            {
                mWindow->adjustTabSize();
            }
            if (emoteWindow)
            {
                emoteWindow->hide();
            }
        }

        void unprotectFocus()
        { mFocusGaining = true; }

        void setVisible(bool visible)
        {
            TextField::setVisible(visible);
        }

    private:
        ChatWindow *mWindow;
        bool mFocusGaining;
};

#endif  // GUI_WIDGETS_CHATINPUT_H
