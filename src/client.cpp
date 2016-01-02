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

#include "client.h"

#include "chatlogger.h"
#include "configmanager.h"
#include "dirs.h"
#include "eventsmanager.h"
#include "game.h"
#include "graphicsmanager.h"
#include "party.h"
#include "settings.h"
#include "soundmanager.h"
#include "spellmanager.h"
#include "statuseffect.h"
#include "units.h"

#include "being/playerinfo.h"
#include "being/playerrelations.h"

#include "const/net/net.h"

#include "input/inputmanager.h"
#include "input/joystick.h"
#include "input/keyboardconfig.h"

#include "input/touch/touchmanager.h"

#include "gui/dialogsmanager.h"
#include "gui/gui.h"
#include "gui/skin.h"
#include "gui/popupmanager.h"
#include "gui/windowmanager.h"

#include "gui/shortcut/dropshortcut.h"
#include "gui/shortcut/emoteshortcut.h"
#include "gui/shortcut/itemshortcut.h"
#include "gui/shortcut/spellshortcut.h"

#include "gui/windows/changeemaildialog.h"
#include "gui/windows/changepassworddialog.h"
#include "gui/windows/charselectdialog.h"
#include "gui/windows/connectiondialog.h"
#include "gui/windows/equipmentwindow.h"
#include "gui/windows/logindialog.h"
#include "gui/windows/npcdialog.h"
#include "gui/windows/okdialog.h"
#include "gui/windows/registerdialog.h"
#include "gui/windows/serverdialog.h"
#include "gui/windows/setupwindow.h"
#include "gui/windows/unregisterdialog.h"
#include "gui/windows/updaterwindow.h"
#include "gui/windows/quitdialog.h"
#include "gui/windows/worldselectdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/createwidget.h"
#include "gui/widgets/desktop.h"
#include "gui/widgets/windowcontainer.h"

#include "net/charserverhandler.h"
#include "net/chathandler.h"
#include "net/download.h"
#include "net/gamehandler.h"
#include "net/generalhandler.h"
#include "net/guildhandler.h"
#include "net/inventoryhandler.h"
#include "net/ipc.h"
#include "net/loginhandler.h"
#include "net/net.h"
#include "net/updatetypeoperators.h"
#include "net/packetlimiter.h"
#include "net/partyhandler.h"

#ifdef TMWA_SUPPORT
#include "net/tmwa/guildmanager.h"
#endif

#include "particle/particle.h"

#include "resources/imagehelper.h"
#include "resources/resourcemanager.h"

#include "resources/sprite/spritereference.h"

#include "resources/db/avatardb.h"
#include "resources/db/badgesdb.h"
#include "resources/db/chardb.h"
#include "resources/db/colordb.h"
#include "resources/db/deaddb.h"
#include "resources/db/emotedb.h"
#include "resources/db/homunculusdb.h"
#include "resources/db/horsedb.h"
#include "resources/db/itemdb.h"
#include "resources/db/sounddb.h"
#include "resources/db/mapdb.h"
#include "resources/db/mercenarydb.h"
#include "resources/db/moddb.h"
#include "resources/db/monsterdb.h"
#include "resources/db/npcdb.h"
#include "resources/db/npcdialogdb.h"
#include "resources/db/palettedb.h"
#include "resources/db/petdb.h"
#include "resources/db/skillunitdb.h"
#include "resources/db/weaponsdb.h"

#include "utils/cpu.h"
#include "utils/delete2.h"
#include "utils/env.h"
#include "utils/fuzzer.h"
#include "utils/gettext.h"
#include "utils/gettexthelper.h"
#ifdef ANDROID
#include "utils/paths.h"
#endif
#include "utils/physfstools.h"
#include "utils/sdlcheckutils.h"
#include "utils/timer.h"

#include "utils/translation/translationmanager.h"

#include "listeners/errorlistener.h"

#ifdef USE_OPENGL
#include "test/testlauncher.h"
#include "test/testmain.h"
#else
#include "configuration.h"
#endif

#ifdef WIN32
#include <SDL_syswm.h>
#include "utils/specialfolder.h"
#undef ERROR
#endif

#ifdef ANDROID
#ifndef USE_SDL2
#include <SDL_screenkeyboard.h>
#include <fstream>
#endif
#endif

#include <sys/stat.h>

#ifdef USE_MUMBLE
#include "mumblemanager.h"
#endif

#include "debug.h"

std::string errorMessage;
LoginData loginData;

Client *client = nullptr;

extern FPSmanager fpsManager;

volatile bool runCounters;
bool isSafeMode = false;
int serverVersion = 0;
int packetVersion = 0;
unsigned int tmwServerVersion = 0;
int start_time;
unsigned int mLastHost = 0;
unsigned long mSearchHash = 0;
int textures_count = 0;

namespace
{
    class AccountListener final : public ActionListener
    {
        public:
            void action(const ActionEvent &) override final
            {
                client->setState(State::CHAR_SELECT);
            }
    } accountListener;

    class LoginListener final : public ActionListener
    {
        public:
            void action(const ActionEvent &) override final
            {
                client->setState(State::PRE_LOGIN);
            }
    } loginListener;
}  // namespace

Client::Client() :
    ActionListener(),
    mCurrentServer(),
    mGame(nullptr),
    mCurrentDialog(nullptr),
    mQuitDialog(nullptr),
    mSetupButton(nullptr),
    mVideoButton(nullptr),
    mHelpButton(nullptr),
    mAboutButton(nullptr),
    mThemesButton(nullptr),
    mPerfomanceButton(nullptr),
#ifdef ANDROID
    mCloseButton(nullptr),
#endif
    mState(State::CHOOSE_SERVER),
    mOldState(State::START),
    mSkin(nullptr),
    mButtonPadding(1),
    mButtonSpacing(3),
    mPing(0),
    mConfigAutoSaved(false)
{
    WindowManager::init();
}

void Client::testsInit()
{
    if (!settings.options.test.empty() &&
        settings.options.test != "99")
    {
        gameInit();
    }
    else
    {
        logger = new Logger;
        Dirs::initLocalDataDir();
        Dirs::initTempDir();
        Dirs::initConfigDir();
    }
}

void Client::gameInit()
{
    logger = new Logger;

    // Load branding information
    if (!settings.options.brandingPath.empty())
        branding.init(settings.options.brandingPath);
    branding.setDefaultValues(getBrandingDefaults());

    Dirs::initRootDir();
    Dirs::initHomeDir();

    // Configure logger
    if (!settings.options.logFileName.empty())
        settings.logFileName = settings.options.logFileName;
    else
        settings.logFileName = settings.localDataDir + "/manaplus.log";
    logger->setLogFile(settings.logFileName);

#ifdef USE_FUZZER
    Fuzzer::init();
#endif
    ConfigManager::backupConfig("config.xml");
    ConfigManager::initConfiguration();
    Net::loadIgnorePackets();
    paths.setDefaultValues(getPathsDefaults());
    initFeatures();
    logger->log("init 4");
    logger->setDebugLog(config.getBoolValue("debugLog"));
    logger->setReportUnimplimented(config.getBoolValue("unimplimentedLog"));

    config.incValue("runcount");

#ifndef ANDROID
    if (settings.options.test.empty())
        ConfigManager::storeSafeParameters();
#endif

    ResourceManager::init();
    if (!resourceManager->setWriteDir(settings.localDataDir))
    {
        logger->error(strprintf("%s couldn't be set as home directory! "
            "Exiting.", settings.localDataDir.c_str()));
    }

    GettextHelper::initLang();

    chatLogger = new ChatLogger;
    if (settings.options.chatLogDir.empty())
    {
        chatLogger->setBaseLogDir(settings.localDataDir
            + std::string("/logs/"));
    }
    else
    {
        chatLogger->setBaseLogDir(settings.options.chatLogDir);
    }

    logger->setLogToStandardOut(config.getBoolValue("logToStandardOut"));

    // Log the client version
    logger->log1(FULL_VERSION);
    logger->log("Start configPath: " + config.getConfigPath());

    Dirs::initScreenshotDir();

    // Initialize SDL
    logger->log1("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        logger->safeError(strprintf("Could not initialize SDL: %s",
            SDL_GetError()));
    }
    atexit(SDL_Quit);

    PacketLimiter::initPacketLimiter();
#ifndef USE_SDL2
    SDL_EnableUNICODE(1);
#endif
    WindowManager::applyKeyRepeat();

    // disable unused SDL events
#ifndef USE_SDL2
    SDL_EventState(SDL_VIDEOEXPOSE, SDL_IGNORE);
#endif
    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

#ifdef WIN32
    Dirs::extractDataDir();
    Dirs::mountDataDir();
#endif

    WindowManager::setIcon();
    ConfigManager::checkConfigVersion();
    logVars();
    Cpu::detect();
#if defined(USE_OPENGL) 
#if !defined(ANDROID) && !defined(__APPLE__) && !defined(__native_client__)
    if (!settings.options.safeMode &&
        settings.options.test.empty() &&
        !config.getBoolValue("videodetected"))
    {
        graphicsManager.detectVideoSettings();
    }
#endif
#endif
    updateEnv();
    initGraphics();

#ifndef WIN32
    Dirs::extractDataDir();
    Dirs::mountDataDir();
#endif

    Dirs::updateDataPath();

    // Add the main data directories to our PhysicsFS search path
    if (!settings.options.dataPath.empty())
    {
        resourceManager->addToSearchPath(settings.options.dataPath,
            Append_false);
    }

    // Add the local data directory to PhysicsFS search path
    resourceManager->addToSearchPath(settings.localDataDir, Append_false);
    TranslationManager::loadCurrentLang();

    WindowManager::initTitle();

    mainGraphics->postInit();

    theme = new Theme;
    Theme::selectSkin();
    touchManager.init();

    // Initialize the item and emote shortcuts.
    for (unsigned f = 0; f < SHORTCUT_TABS; f ++)
        itemShortcut[f] = new ItemShortcut(f);
    emoteShortcut = new EmoteShortcut;
    dropShortcut = new DropShortcut;

    gui = new Gui();
    gui->postInit(mainGraphics);
    dialogsManager = new DialogsManager;
    popupManager = new PopupManager;

    initSoundManager();
    eventsManager.init();

    // Initialize keyboard
    keyboard.init();
    inputManager.init();

    // Initialise player relations
    player_relations.init();
    Joystick::init();
    WindowManager::createWindows();

    keyboard.update();
    if (joystick)
        joystick->update();

    // Initialize default server
    mCurrentServer.hostname = settings.options.serverName;
    mCurrentServer.port = settings.options.serverPort;
    if (!settings.options.serverType.empty())
    {
        mCurrentServer.type = ServerInfo::parseType(
            settings.options.serverType);
    }

    loginData.username = settings.options.username;
    loginData.password = settings.options.password;
    LoginDialog::savedPassword = settings.options.password;
    loginData.remember = serverConfig.getValue("remember", 1);
    loginData.registerLogin = false;

    if (mCurrentServer.hostname.empty())
    {
        mCurrentServer.hostname = branding.getValue("defaultServer", "");
        settings.options.serverName = mCurrentServer.hostname;
    }

    if (mCurrentServer.port == 0)
    {
        mCurrentServer.port = static_cast<uint16_t>(branding.getValue(
            "defaultPort", static_cast<int>(DEFAULT_PORT)));
        mCurrentServer.type = ServerInfo::parseType(
            branding.getValue("defaultServerType", "tmwathena"));
    }

    chatLogger->setServerName(mCurrentServer.hostname);

    if (loginData.username.empty() && loginData.remember)
        loginData.username = serverConfig.getValue("username", "");

    if (mState != State::ERROR)
        mState = State::CHOOSE_SERVER;

    startTimers();

    const int fpsLimit = config.getIntValue("fpslimit");
    settings.limitFps = fpsLimit > 0;

    SDL_initFramerate(&fpsManager);
    WindowManager::setFramerate(fpsLimit);
    initConfigListeners();

    settings.guiAlpha = config.getFloatValue("guialpha");
    optionChanged("fpslimit");

    start_time = static_cast<int>(time(nullptr));

    PlayerInfo::init();

#ifdef ANDROID
#ifndef USE_SDL2
    WindowManager::updateScreenKeyboard(SDL_GetScreenKeyboardHeight(nullptr));
#endif
#endif

#ifdef USE_MUMBLE
    if (!mumbleManager)
        mumbleManager = new MumbleManager();
#endif

    mSkin = theme->load("windowmenu.xml", "");
    if (mSkin)
    {
        mButtonPadding = mSkin->getPadding();
        mButtonSpacing = mSkin->getOption("spacing", 3);
    }
}

Client::~Client()
{
    if (!settings.options.testMode)
        gameClear();
    else
        testsClear();
    CHECKLISTENERS
}

void Client::initConfigListeners()
{
    config.addListener("fpslimit", this);
    config.addListener("guialpha", this);
    config.addListener("gamma", this);
    config.addListener("enableGamma", this);
    config.addListener("particleEmitterSkip", this);
    config.addListener("vsync", this);
    config.addListener("repeateDelay", this);
    config.addListener("repeateInterval", this);
    config.addListener("logInput", this);
}

void Client::initSoundManager()
{
    // Initialize sound engine
    try
    {
        if (config.getBoolValue("sound"))
            soundManager.init();

        soundManager.setSfxVolume(config.getIntValue("sfxVolume"));
        soundManager.setMusicVolume(config.getIntValue("musicVolume"));
    }
    catch (const char *const err)
    {
        mState = State::ERROR;
        errorMessage = err;
        logger->log("Warning: %s", err);
    }
    soundManager.playMusic(branding.getValue(
        "loginMusic", "Magick - Real.ogg"));
}

void Client::initGraphics()
{
    WindowManager::applyVSync();
    runCounters = config.getBoolValue("packetcounters");

    graphicsManager.initGraphics();

    imageHelper->postInit();
    getConfigDefaults2(config.getDefaultValues());
    WindowManager::applyGrabMode();
    WindowManager::applyGamma();

    mainGraphics->beginDraw();
}

void Client::testsClear()
{
    if (!settings.options.test.empty())
        gameClear();
    else
        BeingInfo::clear();
}

void Client::gameClear()
{
    if (logger)
        logger->log1("Quitting1");
    config.removeListeners(this);

    eventsManager.shutdown();
    WindowManager::deleteWindows();
    if (windowContainer)
        windowContainer->slowLogic();

    stopTimers();

    // Unload XML databases
    CharDB::unload();
    DeadDB::unload();
    ColorDB::unload();
    SoundDB::unload();
    EmoteDB::unload();
    HorseDB::unload();
    ItemDB::unload();
    MercenaryDB::unload();
    HomunculusDB::unload();
    MonsterDB::unload();
    NPCDB::unload();
    SkillUnitDb::unload();
    NpcDialogDB::unload();
    AvatarDB::unload();
    BadgesDB::unload();
    WeaponsDB::unload();
    PaletteDB::unload();
    PETDB::unload();
    StatusEffect::unload();
    ModDB::unload();

    if (loginHandler)
        loginHandler->clearWorlds();

    if (chatHandler)
        chatHandler->clear();

    if (charServerHandler)
        charServerHandler->clear();

    delete2(ipc);

#ifdef USE_MUMBLE
    delete2(mumbleManager);
#endif

    PlayerInfo::deinit();

    // Before config.write() since it writes the shortcuts to the config
    for (unsigned f = 0; f < SHORTCUT_TABS; f ++)
        delete2(itemShortcut[f])
    delete2(emoteShortcut);
    delete2(dropShortcut);

    player_relations.store();

    if (logger)
        logger->log1("Quitting2");

    delete2(mCurrentDialog);
    delete2(popupManager);
    delete2(dialogsManager);
    delete2(gui);

    if (inventoryHandler)
        inventoryHandler->clear();

    if (logger)
        logger->log1("Quitting3");

    graphicsManager.deleteRenderers();

    if (logger)
        logger->log1("Quitting4");

    XML::cleanupXML();

    if (logger)
        logger->log1("Quitting5");

    BeingInfo::clear();

    // Shutdown sound
    soundManager.close();

    if (logger)
        logger->log1("Quitting6");

    ActorSprite::unload();

    touchManager.clear();
    ResourceManager::deleteInstance();

    if (logger)
        logger->log1("Quitting8");

    WindowManager::deleteIcon();

    if (logger)
        logger->log1("Quitting9");

    delete2(joystick);

    keyboard.deinit();

    if (logger)
        logger->log1("Quitting10");

    soundManager.shutdown();
    touchManager.shutdown();

#ifdef DEBUG_CONFIG
    config.enableKeyLogging();
#endif
    config.removeOldKeys();
    config.write();
    serverConfig.write();

    config.clear();
    serverConfig.clear();

    if (logger)
        logger->log1("Quitting11");

#ifdef USE_PROFILER
    Perfomance::clear();
#endif

#ifdef DEBUG_OPENGL_LEAKS
    if (logger)
        logger->log("textures left: %d", textures_count);
#endif

    delete2(chatLogger);
    TranslationManager::close();
}

int Client::testsExec()
{
#ifdef USE_OPENGL
    if (settings.options.test.empty())
    {
        TestMain test;
        return test.exec();
    }
    else
    {
        TestLauncher launcher(settings.options.test);
        return launcher.exec();
    }
#else
    return 0;
#endif
}

#define ADDBUTTON(var, object) var = object; \
    x -= var->getWidth() + mButtonSpacing; \
    var->setPosition(x, mButtonPadding); \
    top->add(var);


void Client::stateConnectGame1()
{
    if (gameHandler &&
        loginHandler &&
        gameHandler->isConnected())
    {
        loginHandler->disconnect();
    }
}

void Client::stateConnectServer1()
{
    if (mOldState == State::CHOOSE_SERVER)
    {
        settings.serverName = mCurrentServer.hostname;
        ConfigManager::initServerConfig(mCurrentServer.hostname);
        PacketLimiter::initPacketLimiter();
        initTradeFilter();
        Dirs::initUsersDir();
        player_relations.init();

        // Initialize the item and emote shortcuts.
        for (unsigned f = 0; f < SHORTCUT_TABS; f ++)
        {
            delete itemShortcut[f];
            itemShortcut[f] = new ItemShortcut(f);
        }
        delete emoteShortcut;
        emoteShortcut = new EmoteShortcut;

        // Initialize the drop shortcuts.
        delete dropShortcut;
        dropShortcut = new DropShortcut;

        initFeatures();
        PlayerInfo::loadData();
        loginData.registerUrl = mCurrentServer.registerUrl;
        if (!mCurrentServer.onlineListUrl.empty())
            settings.onlineListUrl = mCurrentServer.onlineListUrl;
        else
            settings.onlineListUrl = settings.serverName;
        settings.persistentIp = mCurrentServer.persistentIp;
        settings.supportUrl = mCurrentServer.supportUrl;
        settings.updateMirrors = mCurrentServer.updateMirrors;

        if (settings.options.username.empty())
        {
            if (loginData.remember)
                loginData.username = serverConfig.getValue("username", "");
            else
                loginData.username.clear();
        }
        else
        {
            loginData.username = settings.options.username;
        }

        loginData.remember = serverConfig.getValue("remember", 1);
        Net::connectToServer(mCurrentServer);

#ifdef USE_MUMBLE
        if (mumbleManager)
            mumbleManager->setServer(mCurrentServer.hostname);
#endif

#ifdef TMWA_SUPPORT
        GuildManager::init();
#endif

        if (!mConfigAutoSaved)
        {
            mConfigAutoSaved = true;
            config.write();
        }
    }
    else if (mOldState != State::CHOOSE_SERVER &&
             loginHandler &&
             loginHandler->isConnected())
    {
        mState = State::PRE_LOGIN;
    }
}

void Client::stateWorldSelect1()
{
    if (mOldState == State::UPDATE &&
        loginHandler)
    {
        if (loginHandler->getWorlds().size() < 2)
            mState = State::PRE_LOGIN;
    }
}

void Client::stateGame1()
{
    if (!gui)
        return;

    BasicContainer2 *const top = static_cast<BasicContainer2*>(
        gui->getTop());

    if (!top)
        return;

    CREATEWIDGETV(desktop, Desktop, nullptr);
    top->add(desktop);
    int x = top->getWidth() - mButtonPadding;
    ADDBUTTON(mSetupButton, new Button(desktop,
        // TRANSLATORS: setup tab quick button
        _("Setup"), "Setup", this))
    ADDBUTTON(mPerfomanceButton, new Button(desktop,
        // TRANSLATORS: perfoamance tab quick button
        _("Performance"), "Perfomance", this))
    ADDBUTTON(mVideoButton, new Button(desktop,
        // TRANSLATORS: video tab quick button
        _("Video"), "Video", this))
    ADDBUTTON(mThemesButton, new Button(desktop,
        // TRANSLATORS: theme tab quick button
        _("Theme"), "Themes", this))
    ADDBUTTON(mAboutButton, new Button(desktop,
        // TRANSLATORS: theme tab quick button
        _("About"), "about", this))
    ADDBUTTON(mHelpButton, new Button(desktop,
        // TRANSLATORS: theme tab quick button
        _("Help"), "help", this))
#ifdef ANDROID
    ADDBUTTON(mCloseButton, new Button(desktop,
        // TRANSLATORS: close quick button
        _("Close"), "close", this))
#endif
    desktop->setSize(mainGraphics->getWidth(),
        mainGraphics->getHeight());
}

void Client::stateSwitchLogin1()
{
    if (mOldState == State::GAME &&
        gameHandler)
    {
        gameHandler->disconnect();
    }
}

int Client::gameExec()
{
    int lastTickTime = tick_time;

    while (mState != State::EXIT)
    {
        PROFILER_START();
        if (eventsManager.handleEvents())
            continue;

        BLOCK_START("Client::gameExec 3")
        if (generalHandler)
            generalHandler->flushNetwork();
        BLOCK_END("Client::gameExec 3")

        BLOCK_START("Client::gameExec 4")
        if (gui)
            gui->logic();
        cur_time = static_cast<int>(time(nullptr));
        int k = 0;
        while (lastTickTime != tick_time &&
               k < 40)
        {
            if (mGame)
                mGame->logic();
            else if (gui)
                gui->handleInput();

            ++lastTickTime;
            k ++;
        }
        soundManager.logic();

        logic_count += k;
        if (gui)
            gui->slowLogic();
        if (mGame)
            mGame->slowLogic();
        slowLogic();
        BLOCK_END("Client::gameExec 4")

        // This is done because at some point tick_time will wrap.
        lastTickTime = tick_time;

        // Update the screen when application is visible, delay otherwise.
        if (!WindowManager::getIsMinimized())
        {
            frame_count++;
            if (gui)
                gui->draw();
            mainGraphics->updateScreen();
        }
        else
        {
            SDL_Delay(100);
        }

        BLOCK_START("~Client::SDL_framerateDelay")
        if (settings.limitFps)
            SDL_framerateDelay(&fpsManager);
        BLOCK_END("~Client::SDL_framerateDelay")

        BLOCK_START("Client::gameExec 6")
        if (mState == State::CONNECT_GAME)
        {
            stateConnectGame1();
        }
        else if (mState == State::CONNECT_SERVER)
        {
            stateConnectServer1();
        }
        else if (mState == State::WORLD_SELECT)
        {
            stateWorldSelect1();
        }
        else if (mOldState == State::START ||
                 (mOldState == State::GAME && mState != State::GAME))
        {
            stateGame1();
        }
        else if (mState == State::SWITCH_LOGIN)
        {
            stateSwitchLogin1();
        }
        BLOCK_END("Client::gameExec 6")

        if (mState != mOldState)
        {
            BLOCK_START("Client::gameExec 7")
            PlayerInfo::stateChange(mState);

            if (mOldState == State::GAME)
            {
                delete2(mGame);
                Game::clearInstance();
                resourceManager->cleanOrphans();
                Party::clearParties();
                Guild::clearGuilds();
                NpcDialog::clearDialogs();
                if (guildHandler)
                    guildHandler->clear();
                if (partyHandler)
                    partyHandler->clear();
                if (chatLogger)
                    chatLogger->clear();
                if (!settings.options.dataPath.empty())
                    UpdaterWindow::unloadMods(settings.options.dataPath);
                else
                    UpdaterWindow::unloadMods(settings.oldUpdates);
                if (!settings.options.skipUpdate)
                    UpdaterWindow::unloadMods(settings.oldUpdates + "/fix/");
            }
            else if (mOldState == State::CHAR_SELECT)
            {
                if (mState != State::CHANGEPASSWORD && charServerHandler)
                    charServerHandler->clear();
            }

            mOldState = mState;

            // Get rid of the dialog of the previous state
            delete2(mCurrentDialog);

            // State has changed, while the quitDialog was active, it might
            // not be correct anymore
            if (mQuitDialog)
            {
                mQuitDialog->scheduleDelete();
                mQuitDialog = nullptr;
            }
            BLOCK_END("Client::gameExec 7")

            BLOCK_START("Client::gameExec 8")
            switch (mState)
            {
                case State::CHOOSE_SERVER:
                {
                    BLOCK_START("Client::gameExec STATE_CHOOSE_SERVER")
                    logger->log1("State: CHOOSE SERVER");
                    mCurrentServer.supportUrl.clear();
                    settings.supportUrl.clear();
                    if (settings.options.dataPath.empty())
                    {
                        // Add customdata directory
                        resourceManager->searchAndRemoveArchives(
                            "customdata/",
                            "zip");
                    }

                    if (!settings.oldUpdates.empty())
                    {
                        UpdaterWindow::unloadUpdates(settings.oldUpdates);
                        settings.oldUpdates.clear();
                    }

                    if (!settings.options.skipUpdate)
                    {
                        resourceManager->searchAndRemoveArchives(
                            settings.updatesDir +
                            "/local/",
                            "zip");

                        resourceManager->removeFromSearchPath(
                            settings.localDataDir +
                            dirSeparator +
                            settings.updatesDir +
                            "/local/");
                    }

                    resourceManager->clearCache();

                    loginData.clearUpdateHost();
                    serverVersion = 0;
                    packetVersion = 0;
                    tmwServerVersion = 0;

                    // Allow changing this using a server choice dialog
                    // We show the dialog box only if the command-line
                    // options weren't set.
                    if (settings.options.serverName.empty() &&
                        settings.options.serverPort == 0 &&
                        !branding.getValue("onlineServerList", "a").empty())
                    {
                        // Don't allow an alpha opacity
                        // lower than the default value
                        theme->setMinimumOpacity(0.8F);

                        CREATEWIDGETV(mCurrentDialog, ServerDialog,
                            &mCurrentServer,
                            settings.configDir);
                    }
                    else
                    {
                        mState = State::CONNECT_SERVER;

                        // Reset options so that cancelling or connect
                        // timeout will show the server dialog.
                        settings.options.serverName.clear();
                        settings.options.serverPort = 0;
                    }
                    BLOCK_END("Client::gameExec STATE_CHOOSE_SERVER")
                    break;
                }

                case State::CONNECT_SERVER:
                    BLOCK_START("Client::gameExec State::CONNECT_SERVER")
                    logger->log1("State: CONNECT SERVER");
                    loginData.updateHosts.clear();
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Connecting to server"),
                    State::SWITCH_SERVER);
                    TranslationManager::loadCurrentLang();
                    BLOCK_END("Client::gameExec State::CONNECT_SERVER")
                    break;

                case State::PRE_LOGIN:
                    logger->log1("State: PRE_LOGIN");
                    break;

                case State::LOGIN:
                    BLOCK_START("Client::gameExec State::LOGIN")
                    logger->log1("State: LOGIN");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    theme->setMinimumOpacity(0.8F);

                    loginData.updateType = static_cast<UpdateTypeT>(
                        serverConfig.getValue("updateType", 0));

                    mSearchHash = Net::Download::adlerBuffer(
                        const_cast<char*>(mCurrentServer.hostname.c_str()),
                        static_cast<int>(mCurrentServer.hostname.size()));
                    if (settings.options.username.empty() ||
                        settings.options.password.empty())
                    {
                        CREATEWIDGETV(mCurrentDialog, LoginDialog,
                            loginData,
                            mCurrentServer.hostname,
                            &settings.options.updateHost);
                    }
                    else
                    {
                        mState = State::LOGIN_ATTEMPT;
                        // Clear the password so that when login fails, the
                        // dialog will show up next time.
                        settings.options.password.clear();
                    }
                    BLOCK_END("Client::gameExec State::LOGIN")
                    break;

                case State::LOGIN_ATTEMPT:
                    BLOCK_START("Client::gameExec State::LOGIN_ATTEMPT")
                    logger->log1("State: LOGIN ATTEMPT");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Logging in"),
                        State::SWITCH_SERVER);
                    if (loginHandler)
                        loginHandler->loginOrRegister(&loginData);
                    BLOCK_END("Client::gameExec State::LOGIN_ATTEMPT")
                    break;

                case State::WORLD_SELECT:
                    BLOCK_START("Client::gameExec State::WORLD_SELECT")
                    logger->log1("State: WORLD SELECT");
                    {
                        TranslationManager::loadCurrentLang();
                        if (!loginHandler)
                        {
                            BLOCK_END("Client::gameExec State::WORLD_SELECT")
                            break;
                        }
                        Worlds worlds = loginHandler->getWorlds();

                        if (worlds.empty())
                        {
                            // Trust that the netcode knows what it's doing
                            mState = State::UPDATE;
                        }
                        else if (worlds.size() == 1)
                        {
                            loginHandler->chooseServer(
                                0, mCurrentServer.persistentIp);
                            mState = State::UPDATE;
                        }
                        else
                        {
                            CREATEWIDGETV(mCurrentDialog, WorldSelectDialog,
                                worlds);
                            if (settings.options.chooseDefault)
                            {
                                static_cast<WorldSelectDialog*>(mCurrentDialog)
                                    ->action(ActionEvent(nullptr, "ok"));
                            }
                        }
                    }
                    BLOCK_END("Client::gameExec State::WORLD_SELECT")
                    break;

                case State::WORLD_SELECT_ATTEMPT:
                    BLOCK_START("Client::gameExec State::WORLD_SELECT_ATTEMPT")
                    logger->log1("State: WORLD SELECT ATTEMPT");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Entering game world"),
                        State::WORLD_SELECT);
                    BLOCK_END("Client::gameExec State::WORLD_SELECT_ATTEMPT")
                    break;

                case State::UPDATE:
                    BLOCK_START("Client::gameExec State::UPDATE")
                    logger->log1("State: UPDATE");

                    // Determine which source to use for the update host
                    if (!settings.options.updateHost.empty())
                        settings.updateHost = settings.options.updateHost;
                    else
                        settings.updateHost = loginData.updateHost;
                    Dirs::initUpdatesDir();

                    if (!settings.oldUpdates.empty())
                        UpdaterWindow::unloadUpdates(settings.oldUpdates);

                    if (settings.options.skipUpdate)
                    {
                        mState = State::LOAD_DATA;
                        settings.oldUpdates.clear();
                        UpdaterWindow::loadDirMods(settings.options.dataPath);
                    }
                    else if (loginData.updateType & UpdateType::Skip)
                    {
                        settings.oldUpdates = settings.localDataDir
                            + dirSeparator + settings.updatesDir;
                        UpdaterWindow::loadLocalUpdates(settings.oldUpdates);
                        mState = State::LOAD_DATA;
                    }
                    else
                    {
                        settings.oldUpdates = settings.localDataDir +
                            dirSeparator +
                            settings.updatesDir;
                        CREATEWIDGETV(mCurrentDialog, UpdaterWindow,
                            settings.updateHost,
                            settings.oldUpdates,
                            settings.options.dataPath.empty(),
                            loginData.updateType);
                    }
                    BLOCK_END("Client::gameExec State::UPDATE")
                    break;

                case State::LOAD_DATA:
                {
                    BLOCK_START("Client::gameExec State::LOAD_DATA")
                    logger->log1("State: LOAD DATA");

                    // If another data path has been set,
                    // we don't load any other files...
                    if (settings.options.dataPath.empty())
                    {
                        // Add customdata directory
                        resourceManager->searchAndAddArchives(
                            "customdata/",
                            "zip",
                            Append_false);
                    }

                    if (!settings.options.skipUpdate)
                    {
                        resourceManager->searchAndAddArchives(
                            settings.updatesDir + "/local/",
                            "zip",
                            Append_false);

                        resourceManager->addToSearchPath(
                            settings.localDataDir +
                            dirSeparator +
                            settings.updatesDir +
                            "/local/",
                            Append_false);
                    }

                    logger->log("Init paths");
                    paths.init("paths.xml", UseResman_true);
                    paths.setDefaultValues(getPathsDefaults());
                    if (!SpriteReference::Empty)
                    {
                        SpriteReference::Empty = new SpriteReference(
                            paths.getStringValue("spriteErrorFile"),
                            0);
                    }

                    if (!BeingInfo::unknown)
                        BeingInfo::unknown = new BeingInfo;

                    initFeatures();
                    TranslationManager::loadCurrentLang();
                    PlayerInfo::stateChange(mState);

                    delete spellManager;
                    spellManager = new SpellManager;
                    delete spellShortcut;
                    spellShortcut = new SpellShortcut;

                    // Load XML databases
                    CharDB::load();
                    DeadDB::load();
                    PaletteDB::load();
                    ColorDB::load();
                    SoundDB::load();
                    MapDB::load();
                    ItemDB::load();
                    Being::load();
                    MercenaryDB::load();
                    HomunculusDB::load();
                    MonsterDB::load();
                    AvatarDB::load();
                    BadgesDB::load();
                    WeaponsDB::load();
                    NPCDB::load();
                    SkillUnitDb::load();
                    NpcDialogDB::load();
                    PETDB::load();
                    HorseDB::load();
                    EmoteDB::load();
//                    ModDB::load();
                    StatusEffect::load();
                    Units::loadUnits();
                    EquipmentWindow::prepareSlotNames();

                    ActorSprite::load();

                    if (desktop)
                        desktop->reloadWallpaper();

                    mState = State::GET_CHARACTERS;
                    BLOCK_END("Client::gameExec State::LOAD_DATA")
                    break;
                }
                case State::GET_CHARACTERS:
                    BLOCK_START("Client::gameExec State::GET_CHARACTERS")
                    logger->log1("State: GET CHARACTERS");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Requesting characters"),
                        State::SWITCH_SERVER);
                    if (charServerHandler)
                        charServerHandler->requestCharacters();
                    BLOCK_END("Client::gameExec State::GET_CHARACTERS")
                    break;

                case State::CHAR_SELECT:
                    BLOCK_START("Client::gameExec State::CHAR_SELECT")
                    logger->log1("State: CHAR SELECT");
                    // Don't allow an alpha opacity
                    // lower than the default value
                    theme->setMinimumOpacity(0.8F);

                    CREATEWIDGETV(mCurrentDialog, CharSelectDialog,
                        loginData);

                    if (!(static_cast<CharSelectDialog*>(mCurrentDialog))
                        ->selectByName(settings.options.character,
                        CharSelectDialog::Choose))
                    {
                        (static_cast<CharSelectDialog*>(mCurrentDialog))
                            ->selectByName(
                            serverConfig.getValue("lastCharacter", ""),
                            settings.options.chooseDefault ?
                            CharSelectDialog::Choose :
                            CharSelectDialog::Focus);
                    }

                    // Choosing character on the command line should work only
                    // once, clear it so that 'switch character' works.
                    settings.options.character.clear();
                    BLOCK_END("Client::gameExec State::CHAR_SELECT")
                    break;

                case State::CONNECT_GAME:
                    BLOCK_START("Client::gameExec State::CONNECT_GAME")
                    logger->log1("State: CONNECT GAME");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Connecting to the game server"),
                        State::CHOOSE_SERVER);
                    if (gameHandler)
                        gameHandler->connect();
                    BLOCK_END("Client::gameExec State::CONNECT_GAME")
                    break;

                case State::CHANGE_MAP:
                    BLOCK_START("Client::gameExec State::CHANGE_MAP")
                    logger->log1("State: CHANGE_MAP");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Changing game servers"),
                        State::SWITCH_CHARACTER);
                    if (gameHandler)
                        gameHandler->connect();
                    BLOCK_END("Client::gameExec State::CHANGE_MAP")
                    break;

                case State::GAME:
                    BLOCK_START("Client::gameExec State::GAME")
                    if (localPlayer)
                    {
                        logger->log("Memorizing selected character %s",
                            localPlayer->getName().c_str());
                        serverConfig.setValue("lastCharacter",
                            localPlayer->getName());
#ifdef USE_MUMBLE
                        if (mumbleManager)
                            mumbleManager->setPlayer(localPlayer->getName());
#endif
                    }

                    // Fade out logon-music here too to give the desired effect
                    // of "flowing" into the game.
                    soundManager.fadeOutMusic(1000);

                    // Allow any alpha opacity
                    theme->setMinimumOpacity(-1.0F);

                    if (chatLogger)
                        chatLogger->setServerName(settings.serverName);

#ifdef ANDROID
                    delete2(mCloseButton);
#endif
                    delete2(mSetupButton);
                    delete2(mVideoButton);
                    delete2(mThemesButton);
                    delete2(mAboutButton);
                    delete2(mHelpButton);
                    delete2(mPerfomanceButton);
                    delete2(desktop);

                    mCurrentDialog = nullptr;

                    logger->log1("State: GAME");
                    if (generalHandler)
                        generalHandler->reloadPartially();
                    mGame = new Game;
                    BLOCK_END("Client::gameExec State::GAME")
                    break;

                case State::LOGIN_ERROR:
                    BLOCK_START("Client::gameExec State::LOGIN_ERROR")
                    logger->log1("State: LOGIN ERROR");
                    CREATEWIDGETV(mCurrentDialog, OkDialog,
                        // TRANSLATORS: error dialog header
                        _("Error"),
                        errorMessage,
                        // TRANSLATORS: ok dialog button
                        _("Close"),
                        DialogType::ERROR,
                        Modal_true,
                        ShowCenter_true,
                        nullptr,
                        260);
                    mCurrentDialog->addActionListener(&loginListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    BLOCK_END("Client::gameExec State::LOGIN_ERROR")
                    break;

                case State::ACCOUNTCHANGE_ERROR:
                    BLOCK_START("Client::gameExec State::ACCOUNTCHANGE_ERROR")
                    logger->log1("State: ACCOUNT CHANGE ERROR");
                    CREATEWIDGETV(mCurrentDialog, OkDialog,
                        // TRANSLATORS: error dialog header
                        _("Error"),
                        errorMessage,
                        // TRANSLATORS: ok dialog button
                        _("Close"),
                        DialogType::ERROR,
                        Modal_true,
                        ShowCenter_true,
                        nullptr,
                        260);
                    mCurrentDialog->addActionListener(&accountListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    BLOCK_END("Client::gameExec State::ACCOUNTCHANGE_ERROR")
                    break;

                case State::REGISTER_PREP:
                    BLOCK_START("Client::gameExec State::REGISTER_PREP")
                    logger->log1("State: REGISTER_PREP");
                    CREATEWIDGETV(mCurrentDialog, ConnectionDialog,
                        // TRANSLATORS: connection dialog header
                        _("Requesting registration details"),
                        State::LOGIN);
                    loginHandler->getRegistrationDetails();
                    BLOCK_END("Client::gameExec State::REGISTER_PREP")
                    break;

                case State::REGISTER:
                    logger->log1("State: REGISTER");
                    CREATEWIDGETV(mCurrentDialog, RegisterDialog,
                        loginData);
                    break;

                case State::REGISTER_ATTEMPT:
                    BLOCK_START("Client::gameExec State::REGISTER_ATTEMPT")
                    logger->log("Username is %s", loginData.username.c_str());
                    if (loginHandler)
                        loginHandler->registerAccount(&loginData);
                    BLOCK_END("Client::gameExec State::REGISTER_ATTEMPT")
                    break;

                case State::CHANGEPASSWORD:
                    BLOCK_START("Client::gameExec State::CHANGEPASSWORD")
                    logger->log1("State: CHANGE PASSWORD");
                    CREATEWIDGETV(mCurrentDialog, ChangePasswordDialog,
                        loginData);
                    mCurrentDialog->setVisible(Visible_true);
                    BLOCK_END("Client::gameExec State::CHANGEPASSWORD")
                    break;

                case State::CHANGEPASSWORD_ATTEMPT:
                    BLOCK_START("Client::gameExec "
                        "State::CHANGEPASSWORD_ATTEMPT")
                    logger->log1("State: CHANGE PASSWORD ATTEMPT");
                    if (loginHandler)
                    {
                        loginHandler->changePassword(loginData.password,
                            loginData.newPassword);
                    }
                    BLOCK_END("Client::gameExec State::CHANGEPASSWORD_ATTEMPT")
                    break;

                case State::CHANGEPASSWORD_SUCCESS:
                    BLOCK_START("Client::gameExec "
                        "State::CHANGEPASSWORD_SUCCESS")
                    logger->log1("State: CHANGE PASSWORD SUCCESS");
                    CREATEWIDGETV(mCurrentDialog, OkDialog,
                        // TRANSLATORS: password change message header
                        _("Password Change"),
                        // TRANSLATORS: password change message text
                        _("Password changed successfully!"),
                        // TRANSLATORS: ok dialog button
                        _("OK"),
                        DialogType::ERROR,
                        Modal_true,
                        ShowCenter_true,
                        nullptr,
                        260);
                    mCurrentDialog->addActionListener(&accountListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    loginData.password = loginData.newPassword;
                    loginData.newPassword.clear();
                    BLOCK_END("Client::gameExec State::CHANGEPASSWORD_SUCCESS")
                    break;

                case State::CHANGEEMAIL:
                    logger->log1("State: CHANGE EMAIL");
                    CREATEWIDGETV(mCurrentDialog,
                        ChangeEmailDialog,
                        loginData);
                    mCurrentDialog->setVisible(Visible_true);
                    break;

                case State::CHANGEEMAIL_ATTEMPT:
                    logger->log1("State: CHANGE EMAIL ATTEMPT");
                    if (loginHandler)
                        loginHandler->changeEmail(loginData.email);
                    break;

                case State::CHANGEEMAIL_SUCCESS:
                    logger->log1("State: CHANGE EMAIL SUCCESS");
                    CREATEWIDGETV(mCurrentDialog, OkDialog,
                        // TRANSLATORS: email change message header
                        _("Email Change"),
                        // TRANSLATORS: email change message text
                        _("Email changed successfully!"),
                        // TRANSLATORS: ok dialog button
                        _("OK"),
                        DialogType::ERROR,
                        Modal_true,
                        ShowCenter_true,
                        nullptr,
                        260);
                    mCurrentDialog->addActionListener(&accountListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    break;

                case State::UNREGISTER:
                    logger->log1("State: UNREGISTER");
                    CREATEWIDGETV(mCurrentDialog,
                        UnRegisterDialog,
                        loginData);
                    break;

                case State::UNREGISTER_ATTEMPT:
                    logger->log1("State: UNREGISTER ATTEMPT");
                    if (loginHandler)
                    {
                        loginHandler->unregisterAccount(
                            loginData.username,
                            loginData.password);
                    }
                    break;

                case State::UNREGISTER_SUCCESS:
                    logger->log1("State: UNREGISTER SUCCESS");
                    if (loginHandler)
                        loginHandler->disconnect();

                    mCurrentDialog = DialogsManager::openErrorDialog(
                        // TRANSLATORS: unregister message header
                        _("Unregister Successful"),
                        // TRANSLATORS: unregister message text
                        _("Farewell, come back any time..."),
                        Modal_true);
                    loginData.clear();
                    // The errorlistener sets the state to State::CHOOSE_SERVER
                    mCurrentDialog->addActionListener(&errorListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    break;

                case State::SWITCH_SERVER:
                    BLOCK_START("Client::gameExec State::SWITCH_SERVER")
                    logger->log1("State: SWITCH SERVER");

                    if (loginHandler)
                        loginHandler->disconnect();
                    if (gameHandler)
                    {
                        gameHandler->disconnect();
                        gameHandler->clear();
                    }
                    settings.serverName.clear();
                    serverConfig.write();
                    serverConfig.unload();
                    if (setupWindow)
                        setupWindow->externalUnload();

                    mState = State::CHOOSE_SERVER;
                    BLOCK_END("Client::gameExec State::SWITCH_SERVER")
                    break;

                case State::SWITCH_LOGIN:
                    BLOCK_START("Client::gameExec State::SWITCH_LOGIN")
                    logger->log1("State: SWITCH LOGIN");

                    if (loginHandler)
                    {
                        loginHandler->logout();
                        loginHandler->disconnect();
                    }
                    if (gameHandler)
                        gameHandler->disconnect();
                    if (loginHandler)
                        loginHandler->connect();

                    mState = State::LOGIN;
                    BLOCK_END("Client::gameExec State::SWITCH_LOGIN")
                    break;

                case State::SWITCH_CHARACTER:
                    BLOCK_START("Client::gameExec State::SWITCH_CHARACTER")
                    logger->log1("State: SWITCH CHARACTER");

                    // Done with game
                    if (gameHandler)
                        gameHandler->disconnect();

                    mState = State::GET_CHARACTERS;
                    BLOCK_END("Client::gameExec State::SWITCH_CHARACTER")
                    break;

                case State::LOGOUT_ATTEMPT:
                    logger->log1("State: LOGOUT ATTEMPT");
                    break;

                case State::WAIT:
                    logger->log1("State: WAIT");
                    break;

                case State::EXIT:
                    BLOCK_START("Client::gameExec State::EXIT")
                    logger->log1("State: EXIT");
                    Net::unload();
                    BLOCK_END("Client::gameExec State::EXIT")
                    break;

                case State::FORCE_QUIT:
                    BLOCK_START("Client::gameExec State::FORCE_QUIT")
                    logger->log1("State: FORCE QUIT");
                    if (generalHandler)
                        generalHandler->unload();
                    mState = State::EXIT;
                    BLOCK_END("Client::gameExec State::FORCE_QUIT")
                  break;

                case State::ERROR:
                    BLOCK_START("Client::gameExec State::ERROR")
                    config.write();
                    if (mOldState == State::GAME)
                        serverConfig.write();
                    logger->log1("State: ERROR");
                    logger->log("Error: %s\n", errorMessage.c_str());
                    // TRANSLATORS: error message header
                    mCurrentDialog = DialogsManager::openErrorDialog(
                        _("Error"),
                        errorMessage,
                        Modal_true);
                    mCurrentDialog->addActionListener(&errorListener);
                    mCurrentDialog = nullptr;  // OkDialog deletes itself
                    gameHandler->disconnect();
                    BLOCK_END("Client::gameExec State::ERROR")
                    break;

                case State::AUTORECONNECT_SERVER:
                    // ++++++
                    break;

                case State::START:
                default:
                    mState = State::FORCE_QUIT;
                    break;
            }
            BLOCK_END("Client::gameExec 8")
        }
        PROFILER_END();
    }

    return 0;
}

void Client::optionChanged(const std::string &name)
{
    if (name == "fpslimit")
    {
        const int fpsLimit = config.getIntValue("fpslimit");
        settings.limitFps = fpsLimit > 0;
        WindowManager::setFramerate(fpsLimit);
    }
    else if (name == "guialpha" ||
             name == "enableGuiOpacity")
    {
        const float alpha = config.getFloatValue("guialpha");
        settings.guiAlpha = alpha;
        ImageHelper::setEnableAlpha(alpha != 1.0F &&
            config.getBoolValue("enableGuiOpacity"));
    }
    else if (name == "gamma" ||
             name == "enableGamma")
    {
        WindowManager::applyGamma();
    }
    else if (name == "particleEmitterSkip")
    {
        Particle::emitterSkip = config.getIntValue("particleEmitterSkip") + 1;
    }
    else if (name == "vsync")
    {
        WindowManager::applyVSync();
    }
    else if (name == "repeateInterval" ||
             name == "repeateDelay")
    {
        WindowManager::applyKeyRepeat();
    }
}

void Client::action(const ActionEvent &event)
{
    std::string tab;
    const std::string &eventId = event.getId();

    if (eventId == "close")
    {
        setState(State::FORCE_QUIT);
        return;
    }
    if (eventId == "Setup")
    {
        tab.clear();
    }
    else if (eventId == "help")
    {
        inputManager.executeAction(InputAction::WINDOW_HELP);
        return;
    }
    else if (eventId == "about")
    {
        inputManager.executeAction(InputAction::WINDOW_ABOUT);
        return;
    }
    else if (eventId == "Video")
    {
        tab = "Video";
    }
    else if (eventId == "Themes")
    {
        tab = "Theme";
    }
    else if (eventId == "Perfomance")
    {
        tab = "Perfomance";
    }
    else
    {
        return;
    }

    if (setupWindow)
    {
        setupWindow->setVisible(fromBool(
            !setupWindow->isWindowVisible(), Visible));
        if (setupWindow->isWindowVisible())
        {
            if (!tab.empty())
                setupWindow->activateTab(tab);
            setupWindow->requestMoveToTop();
        }
    }
}

void Client::initFeatures()
{
    features.init(paths.getStringValue("featuresFile"),
        UseResman_true);
    features.setDefaultValues(getFeaturesDefaults());
    settings.fixDeadAnimation = features.getBoolValue("fixDeadAnimation");
}

void Client::initTradeFilter()
{
    const std::string tradeListName =
        settings.serverConfigDir + "/tradefilter.txt";

    std::ofstream tradeFile;
    struct stat statbuf;

    if (stat(tradeListName.c_str(), &statbuf) ||
        !S_ISREG(statbuf.st_mode))
    {
        tradeFile.open(tradeListName.c_str(),
            std::ios::out);
        if (tradeFile.is_open())
        {
            tradeFile << ": sell" << std::endl;
            tradeFile << ": buy" << std::endl;
            tradeFile << ": trade" << std::endl;
            tradeFile << "i sell" << std::endl;
            tradeFile << "i buy" << std::endl;
            tradeFile << "i trade" << std::endl;
            tradeFile << "i trading" << std::endl;
            tradeFile << "i am buy" << std::endl;
            tradeFile << "i am sell" << std::endl;
            tradeFile << "i am trade" << std::endl;
            tradeFile << "i am trading" << std::endl;
            tradeFile << "i'm buy" << std::endl;
            tradeFile << "i'm sell" << std::endl;
            tradeFile << "i'm trade" << std::endl;
            tradeFile << "i'm trading" << std::endl;
        }
        tradeFile.close();
    }
}

bool Client::isTmw()
{
    const std::string &name = settings.serverName;
    if (name == "server.themanaworld.org" ||
        name == "themanaworld.org" ||
        name == "167.114.129.72")
    {
        return true;
    }
    return false;
}

void Client::moveButtons(const int width)
{
    if (mSetupButton)
    {
        int x = width - mSetupButton->getWidth() - mButtonPadding;
        mSetupButton->setPosition(x, mButtonPadding);
#ifndef WIN32
        x -= mPerfomanceButton->getWidth() + mButtonSpacing;
        mPerfomanceButton->setPosition(x, mButtonPadding);

        x -= mVideoButton->getWidth() + mButtonSpacing;
        mVideoButton->setPosition(x, mButtonPadding);

        x -= mThemesButton->getWidth() + mButtonSpacing;
        mThemesButton->setPosition(x, mButtonPadding);

        x -= mAboutButton->getWidth() + mButtonSpacing;
        mAboutButton->setPosition(x, mButtonPadding);

        x -= mHelpButton->getWidth() + mButtonSpacing;
        mHelpButton->setPosition(x, mButtonPadding);
#ifdef ANDROID
        x -= mCloseButton->getWidth() + mButtonSpacing;
        mCloseButton->setPosition(x, mButtonPadding);
#endif
#endif
    }
}

void Client::windowRemoved(const Window *const window)
{
    if (mCurrentDialog == window)
        mCurrentDialog = nullptr;
}

void Client::logVars()
{
#ifdef ANDROID
    logger->log("APPDIR: %s", getenv("APPDIR"));
    logger->log("DATADIR2: %s", getSdStoragePath().c_str());
#endif
}

void Client::slowLogic()
{
    if (!gameHandler ||
        !gameHandler->mustPing())
    {
        return;
    }

    if (get_elapsed_time1(mPing) > 1500)
    {
        mPing = tick_time;
        if (mState == State::UPDATE ||
            mState == State::LOGIN ||
            mState == State::LOGIN_ATTEMPT)
        {
            if (loginHandler)
                loginHandler->ping();
            if (generalHandler)
                generalHandler->flushSend();
        }
        else if (mState == State::CHAR_SELECT)
        {
            if (charServerHandler)
                charServerHandler->ping();
            if (generalHandler)
                generalHandler->flushSend();
        }
    }
}

#ifdef ANDROID
#ifdef USE_SDL2
/*
void Client::extractAssets()
{
    if (!getenv("APPDIR"))
    {
        logger->log("error: APPDIR is not set!");
        return;
    }
    const std::string fileName = std::string(getenv(
        "APPDIR")).append("/data.zip");
    logger->log("Extracting asset into: " + fileName);
    uint8_t *buf = new uint8_t[1000000];

    FILE *const file = fopen(fileName.c_str(), "w");
    for (int f = 0; f < 100; f ++)
    {
        std::string part = strprintf("manaplus-data.zip%u%u",
            static_cast<unsigned int>(f / 10),
            static_cast<unsigned int>(f % 10));
        logger->log("testing asset: " + part);
        SDL_RWops *const rw = SDL_RWFromFile(part.c_str(), "r");
        if (rw)
        {
            const int size = SDL_RWsize(rw);
            int size2 = SDL_RWread(rw, buf, 1, size);
            logger->log("asset size: %d", size2);
            fwrite(buf, 1, size2, file);
            SDL_RWclose(rw);
            Dirs::setProgress();
        }
        else
        {
            break;
        }
    }
    fclose(file);

    const std::string fileName2 = std::string(getenv(
        "APPDIR")).append("/locale.zip");
    FILE *const file2 = fopen(fileName2.c_str(), "w");
    SDL_RWops *const rw = SDL_RWFromFile("manaplus-locale.zip", "r");
    if (rw)
    {
        const int size = SDL_RWsize(rw);
        int size2 = SDL_RWread(rw, buf, 1, size);
        fwrite(buf, 1, size2, file2);
        SDL_RWclose(rw);
        setProgress();
    }
    fclose(file2);

    delete [] buf;
}
*/
#endif
#endif
