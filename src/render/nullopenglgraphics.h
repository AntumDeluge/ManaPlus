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

#ifndef RENDER_NULLOPENGLGRAPHICS_H
#define RENDER_NULLOPENGLGRAPHICS_H

#if defined USE_OPENGL

#include "localconsts.h"

#include "render/graphics.h"

#include "resources/fboinfo.h"

#ifdef ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#else
#ifndef USE_SDL2
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL_opengl.h>
#if defined(__APPLE__)
#include <OpenGL/glext.h>
#elif !defined(__native_client__)
#include <GL/glext.h>
#endif  // defined(__APPLE__)
#endif

class OpenGLGraphicsVertexes;

class NullOpenGLGraphics final : public Graphics
{
    public:
        NullOpenGLGraphics();

        A_DELETE_COPY(NullOpenGLGraphics)

        ~NullOpenGLGraphics();

        inline void drawQuadArrayfi(const int size) restrict2 A_INLINE;

        inline void drawQuadArrayfi(const GLint *restrict const intVertArray,
                                    const GLfloat *restrict const
                                    floatTexArray,
                                    const int size) restrict2 A_INLINE;

        inline void drawQuadArrayii(const int size) restrict2 A_INLINE;

        inline void drawQuadArrayii(const GLint *restrict const intVertArray,
                                    const GLint *restrict const intTexArray,
                                    const int size) restrict2 A_INLINE;

        inline void drawLineArrayi(const int size) restrict2 A_INLINE;

        inline void drawLineArrayf(const int size) restrict2 A_INLINE;

        #include "render/graphicsdef.hpp"

        #include "render/openglgraphicsdef.hpp"

        #include "render/openglgraphicsdef1.hpp"

        #include "render/openglgraphicsdefadvanced.hpp"

    private:
        GLfloat *mFloatTexArray A_NONNULLPOINTER;
        GLint *mIntTexArray A_NONNULLPOINTER;
        GLint *mIntVertArray A_NONNULLPOINTER;
        bool mTexture;

        bool mIsByteColor;
        Color mByteColor;
        float mFloatColor;
        int mMaxVertices;
        bool mColorAlpha;
#ifdef DEBUG_BIND_TEXTURE
        std::string mOldTexture;
        unsigned mOldTextureId;
#endif
        FBOInfo mFbo;
};
#endif

#endif  // RENDER_NULLOPENGLGRAPHICS_H
