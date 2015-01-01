/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef GUI_WINDOWMANAGER_H
#define GUI_WINDOWMANAGER_H

#include "localconsts.h"

namespace WindowManager
{
    void init();

    bool getIsMinimized() A_WARN_UNUSED;

    void setIsMinimized(const bool n);

    void newChatMessage();

    bool isKeyboardVisible();

    void setFramerate(const int fpsLimit);

    int getFramerate() A_WARN_UNUSED;

    void applyGrabMode();

    void applyGamma();

    void applyVSync();

    void applyKeyRepeat();

    void resizeVideo(int actualWidth,
                     int actualHeight,
                     const bool always);

    void applyScale();

#ifndef USE_SDL2
    void updateScreenKeyboard(const int height);
#endif

    void reloadWallpaper();

    void createWindows();

    void deleteWindows();

    void initTitle();

    void setIcon();

    void deleteIcon();
}  // namespace WindowManager

#endif  // GUI_WINDOWMANAGER_H
