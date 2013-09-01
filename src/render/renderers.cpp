/*
 *  The ManaPlus Client
 *  Copyright (C) 2013  The ManaPlus Developers
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

#include "render/renderers.h"

RenderType intToRenderType(const int mode)
{
    if (mode < 0 || mode > RENDER_LAST)
    {
#ifdef USE_OPENGL
#ifdef ANDROID
        return RENDER_GLES_OPENGL;
#else
        return RENDER_NORMAL_OPENGL;
#endif
#else
        return RENDER_SOFTWARE;
#endif
    }
    return static_cast<RenderType>(mode);
}
