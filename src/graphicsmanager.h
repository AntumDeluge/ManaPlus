/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2015  The ManaPlus Developers
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

#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "main.h"

#ifdef USE_OPENGL

#ifdef ANDROID
#include <GLES/gl.h>
#else
#ifndef USE_SDL2
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL_opengl.h>
// hack to hide warnings
#undef GL_GLEXT_VERSION
#undef GL_GLEXT_PROTOTYPES
#endif

#endif

#include "sdlshared.h"

#include "utils/stringvector.h"

#include <set>
#include <string>

#include "localconsts.h"

class TestMain;

struct FBOInfo;
#ifdef USE_SDL2
struct SDL_Renderer;
#endif
struct SDL_Window;

class GraphicsManager final
{
    public:
        GraphicsManager();

        A_DELETE_COPY(GraphicsManager)

        ~GraphicsManager();

        void createRenderers();

        static void deleteRenderers();

        void initGraphics();

        static void setVideoMode();

        SDL_Window *createWindow(const int w, const int h,
                                 const int bpp, const int flags);

#ifdef USE_SDL2
        SDL_Renderer *createRenderer(SDL_Window *const window,
                                     const int flags);
#endif

        bool getAllVideoModes(StringVect &modeList);

        void detectPixelSize();

        std::string getDensityString() const;

        int getDensity() const
        { return mDensity; }

#ifdef USE_OPENGL
        TestMain *startDetection() A_WARN_UNUSED;

        int detectGraphics() A_WARN_UNUSED;

        bool supportExtension(const std::string &ext) const A_WARN_UNUSED;

        void updateTextureFormat() const;

        bool checkGLVersion(const int major, const int minor)
                            const A_WARN_UNUSED;

        bool checkSLVersion(const int major, const int minor)
                            const A_WARN_UNUSED;

        bool checkPlatformVersion(const int major, const int minor)
                                  const A_WARN_UNUSED;

        static void createFBO(const int width, const int height,
                              FBOInfo *const fbo);

        static void deleteFBO(FBOInfo *const fbo);

        void initOpenGLFunctions();

        void updateExtensions();

        void updatePlanformExtensions();

        void initOpenGL();

        void updateLimits();

        int getMaxVertices() const A_WARN_UNUSED
        { return mMaxVertices; }

        bool getUseAtlases() const A_WARN_UNUSED
        { return mUseAtlases; }

        void logVersion() const;

        void setGLVersion();

        static std::string getGLString(const int num) A_WARN_UNUSED;

        static void logString(const char *const format, const int num);

        void detectVideoSettings();

        void createTextureSampler();

        bool isUseTextureSampler() const A_WARN_UNUSED
        { return mUseTextureSampler; }

        static GLenum getLastError();

        static std::string errorToString(const GLenum error) A_WARN_UNUSED;

        static void logError();

        void updateDebugLog() const;

        std::string getGLVersion() const
        { return mGlVersionString; }

        static GLenum getLastErrorCached()
        { return mLastError; }

        static void resetCachedError()
        { mLastError = GL_NO_ERROR; }
#endif

    private:
        std::set<std::string> mExtensions;

        std::set<std::string> mPlatformExtensions;

        std::string mGlVersionString;

        std::string mGlVendor;

        std::string mGlRenderer;

        std::string mGlShaderVersionString;
#ifdef USE_OPENGL
        static GLenum mLastError;
#endif
        int mMinor;

        int mMajor;

        int mSLMinor;

        int mSLMajor;

        int mPlatformMinor;

        int mPlatformMajor;

        int mMaxVertices;

        int mMaxFboSize;

        uint32_t mMaxWidth;

        uint32_t mMaxHeight;

        uint32_t mWidthMM;

        uint32_t mHeightMM;

        int32_t mDensity;

#ifdef USE_OPENGL
        bool mUseTextureSampler;

        GLuint mTextureSampler;

        int mSupportDebug;

        bool mSupportModernOpengl;
#endif
        bool mUseAtlases;
};

extern GraphicsManager graphicsManager;

#endif  // GRAPHICSMANAGER_H
