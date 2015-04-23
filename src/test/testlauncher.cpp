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

#include "test/testlauncher.h"

#ifdef USE_OPENGL

#include "graphicsmanager.h"
#include "graphicsvertexes.h"
#include "settings.h"
#include "soundmanager.h"

#include "gui/skin.h"
#include "gui/theme.h"

#include "gui/fonts/font.h"

#include "utils/physfscheckutils.h"
#include "utils/physfsrwops.h"

#include "resources/dye.h"
#include "resources/dyepalette.h"
#include "resources/image.h"
#include "resources/imagewriter.h"
#include "resources/openglimagehelper.h"
#include "resources/surfaceimagehelper.h"
#include "resources/wallpaper.h"

#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#endif

#include <sys/time.h>

#include "debug.h"

extern Font *boldFont;

TestLauncher::TestLauncher(std::string test) :
    mTest(test),
    file()
{
    file.open((settings.localDataDir + std::string("/test.log")).c_str(),
        std::ios::out);
}

TestLauncher::~TestLauncher()
{
    file.close();
}

int TestLauncher::exec()
{
    if (mTest == "1" || mTest == "2" || mTest == "3" || mTest == "19")
        return testBackend();
    else if (mTest == "4")
        return testSound();
    else if (mTest == "5" || mTest == "6" || mTest == "7" || mTest == "18")
        return testRescale();
    else if (mTest == "8" || mTest == "9" || mTest == "10" || mTest == "17")
        return testFps();
    else if (mTest == "11")
        return testBatches();
    else if (mTest == "14" || mTest == "15" || mTest == "16" || mTest == "20")
        return testTextures();
    else if (mTest == "99")
        return testVideoDetection();
    else if (mTest == "100")
        return testInternal();
    else if (mTest == "101")
        return testDye();
    else if (mTest == "102")
        return testDraw();
    else if (mTest == "103")
        return testFps2();
    else if (mTest == "104")
        return testFps3();
    else if (mTest == "105")
        return testDyeSpeed();

    return -1;
}

int TestLauncher::testBackend() const
{
    const Image *const img = Theme::getImageFromTheme(
        "graphics/sprites/arrow_up.png");
    if (!img)
        return 1;
    const int cnt = 100;

    for (int f = 0; f < cnt; f ++)
    {
        mainGraphics->drawImage(img, cnt * 7, cnt * 5);
        mainGraphics->updateScreen();
    }

    sleep(1);
    return 0;
}

int TestLauncher::testSound() const
{
    soundManager.playGuiSfx("system/newmessage.ogg");
    sleep(1);
    soundManager.playSfx("system/newmessage.ogg", 0, 0);
    soundManager.playMusic("sfx/system/newmessage.ogg");
    sleep(3);
    soundManager.stopMusic();
    return 0;
}

int TestLauncher::testRescale() const
{
    Wallpaper::loadWallpapers();
    const std::string wallpaperName = Wallpaper::getWallpaper(800, 600);
    const volatile Image *const img = Theme::getImageFromTheme(wallpaperName);
    if (!img)
        return 1;

    sleep(1);
    return 0;
}

int TestLauncher::testFps()
{
    timeval start;
    timeval end;

    Wallpaper::loadWallpapers();
    Wallpaper::getWallpaper(800, 600);
    Image *img[5];
    const int sz = 4;

    img[0] = Theme::getImageFromTheme("graphics/sprites/arrow_up.png");
    img[1] = Theme::getImageFromTheme(
        "graphics/gui/target-cursor-normal-l.png");
    img[2] = Theme::getImageFromTheme("themes/wood/window.png");
    img[3] = Theme::getImageFromTheme("themes/pink/window.png");
    img[4] = Theme::getImageFromTheme("graphics/images/login_wallpaper.png");
    int idx = 0;

    const int cnt = 50;

    gettimeofday(&start, nullptr);
    for (int k = 0; k < cnt; k ++)
    {
        for (int x = 0; x < 800; x += 30)
        {
            for (int y = 0; y < 600; y += 50)
            {
                mainGraphics->drawImage(img[idx], x, y);
                idx ++;
                if (idx > sz)
                    idx = 0;
                mainGraphics->drawImage(img[idx], x, y);
                idx ++;
                if (idx > sz)
                    idx = 0;
            }
        }
        mainGraphics->updateScreen();
    }

    gettimeofday(&end, nullptr);
    const int tFps = calcFps(&start, &end, cnt);
    file << mTest << std::endl;
    file << tFps << std::endl;

    printf("fps: %d\n", tFps / 10);
    sleep(1);
    return 0;
}

int TestLauncher::testFps2()
{
    timeval start;
    timeval end;

    Wallpaper::loadWallpapers();
    Wallpaper::getWallpaper(800, 600);
    Image *img[1];

    img[0] = Theme::getImageFromTheme("graphics/images/login_wallpaper.png");
    mainGraphics->drawImage(img[0], 0, 0);

    const int cnt = 500;

    gettimeofday(&start, nullptr);
    for (int k = 0; k < cnt; k ++)
    {
        for (int f = 0; f < 300; f ++)
            mainGraphics->testDraw();
        mainGraphics->updateScreen();
    }

    gettimeofday(&end, nullptr);
    const int tFps = calcFps(&start, &end, cnt);
    file << mTest << std::endl;
    file << tFps << std::endl;

    printf("fps: %d\n", tFps / 10);
    sleep(1);
    return 0;
}

int TestLauncher::testFps3()
{
    timeval start;
    timeval end;

    Wallpaper::loadWallpapers();
    Wallpaper::getWallpaper(800, 600);
    Image *img[2];

    img[0] = Theme::getImageFromTheme("graphics/sprites/arrow_up.png");
    img[1] = Theme::getImageFromTheme("graphics/sprites/arrow_left.png");
    ImageVertexes *const vert1 = new ImageVertexes;
    vert1->image = img[0];
    ImageVertexes *const vert2 = new ImageVertexes;
    vert2->image = img[1];

    for (int f = 0; f < 50; f ++)
    {
        for (int d = 0; d < 50; d ++)
        {
            mainGraphics->calcTileVertexes(vert1, img[0], f * 16, d * 12);
            mainGraphics->calcTileVertexes(vert1, img[1], f * 16 + 5, d * 12);
        }
    }
    mainGraphics->finalize(vert1);
    mainGraphics->finalize(vert2);

    const int cnt = 2000;

    gettimeofday(&start, nullptr);
    for (int k = 0; k < cnt; k ++)
    {
        mainGraphics->drawTileVertexes(vert1);
        mainGraphics->drawTileVertexes(vert2);
        mainGraphics->updateScreen();
    }

    gettimeofday(&end, nullptr);
    const int tFps = calcFps(&start, &end, cnt);
    file << mTest << std::endl;
    file << tFps << std::endl;

    printf("fps: %d\n", tFps / 10);
    sleep(1);
    return 0;
}

int TestLauncher::testBatches()
{
    int batches = 512;

    file << mTest << std::endl;
    file << batches << std::endl;
    return 0;
}

int TestLauncher::testTextures()
{
    int maxSize = 512;
    int nextSize = 512;
    int sz = OpenGLImageHelper::getTextureSize() + 1;
    if (sz > 16500)
        sz = 16500;

    const uint32_t bytes1[] =
    {
        0xFFFF0000U, 0xFFFFFF00U, 0xFF00FFFFU, 0xFF0000FFU,
        0xFF000000U, 0xFFFF00FFU
    };

    const uint32_t bytes2[] =
    {
        0xFF0000FFU, 0xFF00FFFFU, 0xFFFFFF00U, 0xFFFF0000U,
        0xFF000000U, 0xFFFF00FFU
    };

    for (nextSize = 512; nextSize < sz; nextSize *= 2)
    {
        mainGraphics->clearScreen();
        SDL_Surface *const surface = imageHelper->create32BitSurface(
            nextSize, nextSize);
        if (!surface)
            break;
        uint32_t *pixels = static_cast<uint32_t*>(surface->pixels);
        for (int f = 0; f < 6; f ++)
            pixels[f] = bytes1[f];
        graphicsManager.getLastError();
        graphicsManager.resetCachedError();
        Image *const image = imageHelper->load(surface);
        SDL_FreeSurface(surface);
        if (!image)
            break;

        if (graphicsManager.getLastErrorCached() != GL_NO_ERROR)
        {
            delete image;
            break;
        }
        Image *const subImage = image->getSubImage(0, 0, 10, 10);
        if (!subImage)
        {
            delete image;
            break;
        }
        mainGraphics->drawImage(subImage, 0, 0);
        mainGraphics->updateScreen();
        mainGraphics->drawImage(subImage, 0, 0);
        delete subImage;
        SDL_Surface *const screen1 = mainGraphics->getScreenshot();
        SDL_Surface *const screen2 = imageHelper->convertTo32Bit(screen1);
        SDL_FreeSurface(screen1);
        pixels = static_cast<uint32_t*>(screen2->pixels);
        bool fail(false);
        for (int f = 0; f < 6; f ++)
        {
            if (pixels[f] != bytes2[f])
            {
                fail = true;
                break;
            }
        }

        SDL_FreeSurface(screen2);
        if (fail)
            break;

        maxSize = nextSize;
    }

    file << mTest << std::endl;
    file << maxSize << std::endl;
    printf("OpenGL max size: %d\n", sz);
    printf("actual max size: %d\n", maxSize);
    return 0;
}

int TestLauncher::testInternal()
{
    timeval start;
    timeval end;

    Wallpaper::loadWallpapers();
    Wallpaper::getWallpaper(800, 600);
    Image *img[4];

    img[0] = Theme::getImageFromTheme(
        "graphics/sprites/manaplus_emotions.png");
    img[1] = Theme::getImageFromTheme(
        "graphics/sprites/manaplus_emotions.png");
    img[2] = Theme::getImageFromTheme("graphics/sprites/arrow_left.png");
    img[3] = Theme::getImageFromTheme("graphics/sprites/arrow_right.png");
    int idx = 0;
    const int mem =  mainGraphics->getMemoryUsage();

//    int cnt = 5;
    const int cnt = 5000;

    gettimeofday(&start, nullptr);
    for (int k = 0; k < cnt; k ++)
    {
        for (int x = 0; x < 800; x += 20)
        {
            for (int y = 0; y < 600; y += 25)
            {
                mainGraphics->drawImage(img[idx], x, y);
                mainGraphics->drawImage(img[idx], x + 1, y);
                mainGraphics->drawImage(img[idx], x, y + 5);

                idx ++;
                if (idx > 3)
                    idx = 0;
            }
        }
        mainGraphics->updateScreen();
    }

    gettimeofday(&end, nullptr);
    const int tFps = calcFps(&start, &end, cnt);
    file << mTest << std::endl;
    file << tFps << std::endl;
    file << mem << std::endl;

    sleep(1);
    return 0;
}

int TestLauncher::testDye()
{
    SDL_RWops *rw = MPHYSFSRWOPS_openRead(
        "graphics/sprites/arrow_up.png");
    Dye *d = nullptr;

    if (rw)
    {
        Image *image = d ? surfaceImageHelper->load(rw, *d)
            : surfaceImageHelper->load(rw);
        if (image)
        {
            const SDL_Rect &rect = image->mBounds;
            SDL_Surface *surface = surfaceImageHelper->create32BitSurface(
                rect.w, rect.h);
            if (surface)
            {
                SurfaceImageHelper::combineSurface(image->mSDLSurface, nullptr,
                    surface, nullptr);
                ImageWriter::writePNG(image->mSDLSurface,
                    settings.tempDir + "/testimage1.png");
                ImageWriter::writePNG(surface,
                    settings.tempDir + "/testimage2.png");
            }

            rw = MPHYSFSRWOPS_openRead(
                "graphics/sprites/arrow_up.png");
            d = new Dye("S:#0000ff,00ff00,5c5cff,ff0000");
            image = surfaceImageHelper->load(rw, *d);
            if (image)
            {
                surface = surfaceImageHelper->create32BitSurface(
                    rect.w, rect.h);
                if (surface)
                {
                    SurfaceImageHelper::combineSurface(image->mSDLSurface,
                        nullptr, surface, nullptr);
                    ImageWriter::writePNG(image->mSDLSurface,
                        settings.tempDir + "/testimage3.png");
                    ImageWriter::writePNG(surface,
                        settings.tempDir + "/testimage4.png");
                }
            }
        }
    }
    return 0;
}

int TestLauncher::testDyeSpeed()
{
    timeval start;
    timeval end;
    const int sz = 10000;
    uint32_t buf[sz];

    DyePalette pal("#0000ff,000000,000020,706050", 6);

    for (int f = 0; f < sz; f ++)
        buf[f] = f;

    gettimeofday(&start, nullptr);

    pal.replaceSColor(buf, sz);

    gettimeofday(&end, nullptr);
    printf("time: %f\n", calcTime(&start, &end));
    return 0;
}

int TestLauncher::testDraw()
{
    Image *img[3];
    img[0] = Theme::getImageFromTheme("graphics/sprites/arrow_left.png");
    img[1] = Theme::getImageFromTheme("graphics/sprites/arrow_right.png");
    img[2] = Theme::getImageFromTheme("graphics/sprites/arrow_up.png");
    ImageCollection *const col = new ImageCollection;
    ImageCollection *const col2 = new ImageCollection;
    ImageVertexes *const vert = new ImageVertexes;
    vert->image = img[2];
    Skin *skin = theme->load("button.xml", "button.xml");

    mainGraphics->pushClipArea(Rect(10, 20, 790, 580));
    mainGraphics->setColor(Color(0xFFU, 0xFFU, 0x00U, 0xFFU));
    mainGraphics->drawRectangle(Rect(0, 0, 400, 200));

    mainGraphics->setColor(Color(0xFFU, 0x00U, 0x00U, 0xB0U));
    img[0]->setAlpha(0.5f);
    mainGraphics->drawImage(img[0], 190, 383);
    img[0]->setAlpha(1.0f);

    mainGraphics->calcWindow(col2,
                5, 40,
                500, 40,
                skin->getBorder());
    mainGraphics->finalize(col2);

    mainGraphics->calcTileVertexes(vert, img[2], 10, 10);
    mainGraphics->calcTileVertexes(vert, img[2], 40, 10);
    mainGraphics->finalize(vert);

    mainGraphics->setColor(Color(0x80U, 0x00U, 0xA0U, 0x90U));
    mainGraphics->fillRectangle(Rect(200, 100, 300, 300));
    mainGraphics->popClipArea();

    mainGraphics->setColorAll(Color(0xFFU, 0x00U, 0x00U, 0xB0U),
        Color(0x00U, 0xFFU, 0x00U, 0xB0U));
    boldFont->drawString(mainGraphics,
        "test test test test test test test test ", 300, 100);

    mainGraphics->drawTileCollection(col2);

    mainGraphics->drawPattern(img[0], 10, 400, 300, 180);

    mainGraphics->calcPattern(col, img[1], 500, 400, 150, 100);
    mainGraphics->finalize(col);

    mainGraphics->drawTileVertexes(vert);

    mainGraphics->drawRescaledImage(img[0], 250, 350, 35, 90);

    mainGraphics->setColor(Color(0x00U, 0xFFU, 0x00U, 0x90U));
    mainGraphics->drawNet(450, 10, 600, 300, 32, 20);

    mainGraphics->drawTileCollection(col);

    img[0]->setAlpha(0.3f);
    mainGraphics->drawRescaledPattern(img[0], 250, 150, 250, 300, 30, 100);

    for (int f = 0; f < 255; f ++)
    {
        mainGraphics->setColor(Color(0x20U, 0x60U, f, 0x90U));
        mainGraphics->drawLine(300 + f, 490, 300 + f, 480);
        for (int d = 0; d < 10; d ++)
            mainGraphics->drawPoint(300 + f, 500 + d);
    }
    mainGraphics->updateScreen();
    sleep(10);

    delete col;
    return 0;
}

int TestLauncher::testVideoDetection()
{
    file << mTest << std::endl;
    file << graphicsManager.detectGraphics() << std::endl;
    return 0;
}

int TestLauncher::calcFps(const timeval *const start, const timeval *const end,
                          const int calls) const
{
    long mtime;
    long seconds;
    long useconds;

    seconds  = end->tv_sec  - start->tv_sec;
    useconds = end->tv_usec - start->tv_usec;

    mtime = (seconds * 1000 + useconds / 1000.0) + 0.5;
    if (mtime == 0)
        return 100000;

    return static_cast<int>(static_cast<long>(calls) * 10000 / mtime);
}

float TestLauncher::calcTime(const timeval *const start,
                            const timeval *const end) const
{
    const long seconds = end->tv_sec  - start->tv_sec;
    const long useconds = end->tv_usec - start->tv_usec;

    return (seconds * 1000 + useconds / 1000.0);
}
#endif
