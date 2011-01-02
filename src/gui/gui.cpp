/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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

#include "gui/gui.h"

#include "gui/focushandler.h"
#include "gui/palette.h"
#include "gui/sdlinput.h"
#include "gui/theme.h"
#include "gui/truetypefont.h"

#include "gui/widgets/window.h"
#include "gui/widgets/windowcontainer.h"

#include "configlistener.h"
#include "configuration.h"
#include "graphics.h"
#include "log.h"

#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/imageloader.h"
#include "resources/resourcemanager.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>

// Guichan stuff
Gui *gui = 0;
SDLInput *guiInput = 0;

// Bolded font
gcn::Font *boldFont = 0;

class GuiConfigListener : public ConfigListener
{
    public:
        GuiConfigListener(Gui *g):
            mGui(g)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "customcursor" && mGui)
            {
                bool bCustomCursor = config.getBoolValue("customcursor");
                mGui->setUseCustomCursor(bCustomCursor);
            }
        }
    private:
        Gui *mGui;
};

Gui::Gui(Graphics *graphics):
    mCustomCursor(false),
    mMouseCursors(NULL),
    mMouseCursorAlpha(1.0f),
    mMouseInactivityTimer(0),
    mCursorType(CURSOR_POINTER)
{
    logger->log1("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set image loader
    static ImageLoader imageLoader;
    gcn::Image::setImageLoader(&imageLoader);

    // Set input
    guiInput = new SDLInput;
    setInput(guiInput);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler;

    // Initialize top GUI widget
    WindowContainer *guiTop = new WindowContainer;
    guiTop->setFocusable(true);
    guiTop->setDimension(gcn::Rectangle(0, 0,
        graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    const int fontSize = config.getIntValue("fontSize");
    std::string fontFile = config.getValue("font", "");
// may be here need get paths from paths.getValue?
//    std::string path = resman->getPath(fontFile);
    if (fontFile.empty())
        fontFile = branding.getStringValue("font");

    try
    {
        mGuiFont = new TrueTypeFont(fontFile, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    // Set particle font
    fontFile = config.getValue("particleFont", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("particleFont");

    try
    {
        mInfoParticleFont = new TrueTypeFont(
            fontFile, fontSize, TTF_STYLE_BOLD);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    // Set bold font
    fontFile = config.getValue("boldFont", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("boldFont");

    try
    {
        boldFont = new TrueTypeFont(fontFile, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    // Set help font
    fontFile = config.getValue("helpFont", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("helpFont");

    try
    {
        mHelpFont = new TrueTypeFont(fontFile, fontSize);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load '") + fontFile +
                      std::string("': ") + e.getMessage());
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getBoolValue("customcursor"));
    mConfigListener = new GuiConfigListener(this);
    config.addListener("customcursor", mConfigListener);
}

Gui::~Gui()
{
    config.removeListener("customcursor", mConfigListener);
    delete mConfigListener;
    mConfigListener = 0;

    if (mMouseCursors)
    {
        mMouseCursors->decRef();
        mMouseCursors = 0;
    }

    delete mGuiFont;
    mGuiFont = 0;
    delete boldFont;
    boldFont = 0;
    delete mHelpFont;
    mHelpFont = 0;
    delete mInfoParticleFont;
    mInfoParticleFont = 0;
    delete getTop();

    delete guiInput;
    guiInput = 0;

    Theme::deleteInstance();
}

void Gui::logic()
{
    ResourceManager *resman = ResourceManager::getInstance();
    resman->clearScheduled();

    // Fade out mouse cursor after extended inactivity
    if (mMouseInactivityTimer < 100 * 15)
    {
        ++mMouseInactivityTimer;
        mMouseCursorAlpha = std::min(1.0f, mMouseCursorAlpha + 0.05f);
    }
    else
    {
        mMouseCursorAlpha = std::max(0.0f, mMouseCursorAlpha - 0.005f);
    }

    Palette::advanceGradients();

    gcn::Gui::logic();
}

void Gui::draw()
{
    mGraphics->pushClipArea(getTop()->getDimension());
    getTop()->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1))
        && mMouseCursors && mCustomCursor && mMouseCursorAlpha > 0.0f)
    {
        Image *mouseCursor = mMouseCursors->get(mCursorType);
        if (mouseCursor)
        {
            mouseCursor->setAlpha(mMouseCursorAlpha);

            static_cast<Graphics*>(mGraphics)->drawImage(
                    mouseCursor,
                    mouseX - 15,
                    mouseY - 17);
        }
    }

    mGraphics->popClipArea();
}

void Gui::setUseCustomCursor(bool customCursor)
{
    if (customCursor != mCustomCursor)
    {
        mCustomCursor = customCursor;

        if (mCustomCursor)
        {
            // Hide the SDL mouse cursor
            SDL_ShowCursor(SDL_DISABLE);

            // Load the mouse cursor
            mMouseCursors = Theme::getImageSetFromTheme("mouse.png", 40, 40);

            if (!mMouseCursors)
                logger->log("Error: Unable to load mouse cursors.");
        }
        else
        {
            // Show the SDL mouse cursor
            SDL_ShowCursor(SDL_ENABLE);

            // Unload the mouse cursor
            if (mMouseCursors)
            {
                mMouseCursors->decRef();
                mMouseCursors = NULL;
            }
        }
    }
}

void Gui::handleMouseMoved(const gcn::MouseInput &mouseInput)
{
    gcn::Gui::handleMouseMoved(mouseInput);
    mMouseInactivityTimer = 0;
}

void Gui::updateFonts()
{
    const int fontSize = config.getIntValue("fontSize");
    std::string fontFile = config.getValue("font", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("font");

    static_cast<TrueTypeFont*>(mGuiFont)->loadFont(fontFile, fontSize);

    fontFile = config.getValue("particleFont", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("particleFont");

    static_cast<TrueTypeFont*>(mInfoParticleFont)->loadFont(
        fontFile, fontSize, TTF_STYLE_BOLD);

    fontFile = config.getValue("boldFont", "");
    if (fontFile.empty())
        fontFile = branding.getStringValue("boldFont");

    static_cast<TrueTypeFont*>(boldFont)->loadFont(fontFile, fontSize);
}
