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

#ifndef RENDER_MODERNOPENGLGRAPHICS_H
#define RENDER_MODERNOPENGLGRAPHICS_H

#if defined(USE_OPENGL) && !defined(ANDROID) && !defined(__native_client__)

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
#ifndef __native_client__
#include <GL/glext.h>
#endif
#endif

class OpenGLGraphicsVertexes;
class ShaderProgram;

class ModernOpenGLGraphics final : public Graphics
{
    public:
        ModernOpenGLGraphics();

        A_DELETE_COPY(ModernOpenGLGraphics)

        ~ModernOpenGLGraphics();

        void postInit() restrict2 final;

        void setColor(const Color &restrict color) restrict2 final;

        void screenResized() restrict2 final;

        void finalize(ImageCollection *restrict const col)
                      restrict2 final;

        void finalize(ImageVertexes *restrict const vert)
                      restrict2 final;

        void testDraw() restrict2 final;

        void removeArray(const uint32_t id,
                         uint32_t *restrict const arr)
                         restrict2 final A_NONNULL(3);

        void createGLContext() restrict2 final;

        #include "render/graphicsdef.hpp"

        #include "render/openglgraphicsdef.hpp"

        #include "render/openglgraphicsdefadvanced.hpp"

    private:
        void deleteGLObjects() restrict2;

        inline void drawQuad(const int srcX,
                             const int srcY,
                             const int dstX,
                             const int dstY,
                             const int width,
                             const int height) restrict2 A_INLINE;

        inline void drawRescaledQuad(const int srcX, const int srcY,
                                     const int dstX, const int dstY,
                                     const int width, const int height,
                                     const int desiredWidth,
                                     const int desiredHeight)
                                     restrict2 A_INLINE;

        inline void drawTriangleArray(const int size) restrict2 A_INLINE;

        inline void drawTriangleArray(const GLint *restrict const array,
                                      const int size) restrict2 A_INLINE;

        inline void drawLineArrays(const int size) restrict2 A_INLINE;

        inline void bindArrayBuffer(const GLuint vbo) restrict2 A_INLINE;

        inline void bindArrayBufferAndAttributes(const GLuint vbo)
                                                 restrict2 A_INLINE;

        inline void bindAttributes() restrict2 A_INLINE;

        inline void bindElementBuffer(const GLuint ebo) restrict2 A_INLINE;

        GLint *mIntArray A_NONNULLPOINTER;
        GLint *mIntArrayCached A_NONNULLPOINTER;
        ShaderProgram *mProgram;
        float mAlphaCached;
        int mVpCached;

        float mFloatColor;
        int mMaxVertices;
        GLuint mProgramId;
        GLuint mSimpleColorUniform;
        GLint mPosAttrib;
        GLint mTextureColorUniform;
        GLuint mScreenUniform;
        GLuint mDrawTypeUniform;
        GLuint mVao;
        GLuint mVbo;
        GLuint mEbo;
        GLuint mVboBinded;
        GLuint mEboBinded;
        GLuint mAttributesBinded;
        bool mColorAlpha;
        bool mTextureDraw;
#ifdef DEBUG_BIND_TEXTURE
        std::string mOldTexture;
        unsigned mOldTextureId;
#endif
        FBOInfo mFbo;
};
#endif  // defined(USE_OPENGL) && !defined(ANDROID) &&
        // !defined(__native_client__)

#endif  // RENDER_MODERNOPENGLGRAPHICS_H
