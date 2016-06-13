/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2016  The ManaPlus Developers
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

#include "actions/actions.h"

#include "actormanager.h"
#include "configuration.h"
#include "game.h"
#ifdef USE_OPENGL
#include "graphicsmanager.h"
#endif
#include "main.h"
#include "spellmanager.h"

#include "actions/actiondef.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "const/spells.h"

#include "const/resources/skill.h"

#include "gui/gui.h"
#include "gui/popupmanager.h"
#include "gui/sdlinput.h"
#include "gui/windowmanager.h"

#include "gui/shortcut/dropshortcut.h"
#include "gui/shortcut/emoteshortcut.h"
#include "gui/shortcut/itemshortcut.h"

#include "gui/popups/popupmenu.h"

#include "gui/windows/buydialog.h"
#include "gui/windows/okdialog.h"
#include "gui/windows/tradewindow.h"
#include "gui/windows/quitdialog.h"
#include "gui/windows/buyselldialog.h"
#include "gui/windows/chatwindow.h"
#include "gui/windows/helpwindow.h"
#include "gui/windows/inventorywindow.h"
#include "gui/windows/itemamountwindow.h"
#include "gui/windows/npcdialog.h"
#include "gui/windows/outfitwindow.h"
#include "gui/windows/setupwindow.h"
#include "gui/windows/shopwindow.h"
#include "gui/windows/shortcutwindow.h"
#include "gui/windows/skilldialog.h"
#include "gui/windows/whoisonline.h"

#include "gui/widgets/createwidget.h"

#include "gui/widgets/tabs/chat/chattab.h"

#include "input/inputactionoperators.h"

#if defined USE_OPENGL
#include "render/normalopenglgraphics.h"
#endif

#include "net/adminhandler.h"
#include "net/beinghandler.h"
#ifdef EATHENA_SUPPORT
#include "net/buyingstorehandler.h"
#endif
#include "net/buysellhandler.h"
#include "net/chathandler.h"
#include "net/download.h"
#include "net/gamehandler.h"
#include "net/inventoryhandler.h"
#include "net/ipc.h"
#ifdef EATHENA_SUPPORT
#include "net/mercenaryhandler.h"
#include "net/net.h"
#include "net/vendinghandler.h"
#endif
#include "net/npchandler.h"
#include "net/playerhandler.h"
#include "net/serverfeatures.h"
#include "net/uploadcharinfo.h"
#include "net/tradehandler.h"

#include "resources/iteminfo.h"
#include "resources/memorymanager.h"

#include "resources/resourcemanager/resourcemanager.h"

#include "utils/chatutils.h"
#include "utils/delete2.h"
#include "utils/gettext.h"
#include "utils/files.h"
#include "utils/timer.h"
#include "utils/mathutils.h"

#ifdef ANDROID
#ifndef USE_SDL2
#include <SDL_screenkeyboard.h>
#endif
#endif

#include <sstream>

#include "debug.h"

extern std::string tradePartnerName;
extern QuitDialog *quitDialog;
extern int start_time;
extern char **environ;

namespace Actions
{

static int uploadUpdate(void *ptr,
                        const DownloadStatusT status,
                        size_t total A_UNUSED,
                        const size_t remaining A_UNUSED) A_NONNULL(1);

static int uploadUpdate(void *ptr,
                        const DownloadStatusT status,
                        size_t total A_UNUSED,
                        const size_t remaining A_UNUSED)
{
    if (status == DownloadStatus::Idle || status == DownloadStatus::Starting)
        return 0;

    UploadChatInfo *const info = reinterpret_cast<UploadChatInfo*>(ptr);
    if (!info)
        return 0;

    if (status == DownloadStatus::Complete)
    {
        std::string str = Net::Download::getUploadResponse();
        const size_t sz = str.size();
        if (sz > 0)
        {
            if (str[sz - 1] == '\n')
                str = str.substr(0, sz - 1);
            str.append(info->addStr);
            ChatTab *const tab = info->tab;
            if (chatWindow && (!tab || chatWindow->isTabPresent(tab)))
            {
                str = strprintf("%s [@@%s |%s@@]",
                    info->text.c_str(), str.c_str(), str.c_str());
                outStringNormal(tab, str, str);
            }
            else
            {
                CREATEWIDGET(OkDialog,
                    // TRANSLATORS: file uploaded message
                    _("File uploaded"),
                    str,
                    // TRANSLATORS: ok dialog button
                    _("OK"),
                    DialogType::OK,
                    Modal_true,
                    ShowCenter_false,
                    nullptr,
                    260);
            }
        }
    }
    delete2(info->upload);
    delete info;
    return 0;
}

static void uploadFile(const std::string &str,
                       const std::string &fileName,
                       const std::string &addStr,
                       ChatTab *const tab)
{
    UploadChatInfo *const info = new UploadChatInfo;
    Net::Download *const upload = new Net::Download(info,
        "http://sprunge.us",
        &uploadUpdate,
        false, true, false);
    info->upload = upload;
    info->text = str;
    info->addStr = addStr;
    info->tab = tab;
    upload->setFile(fileName);
    upload->start();
}

static Being *findBeing(const std::string &name, const bool npc)
{
    if (!localPlayer || !actorManager)
        return nullptr;

    Being *being = nullptr;

    if (name.empty())
    {
        being = localPlayer->getTarget();
    }
    else
    {
        being = actorManager->findBeingByName(
            name, ActorType::Unknown);
    }
    if (!being && npc)
    {
        being = actorManager->findNearestLivingBeing(
            localPlayer, 1, ActorType::Npc, AllowSort_true);
        if (being)
        {
            if (abs(being->getTileX() - localPlayer->getTileX()) > 1
                || abs(being->getTileY() - localPlayer->getTileY()) > 1)
            {
                being = nullptr;
            }
        }
    }
    if (!being && npc)
    {
        being = actorManager->findNearestLivingBeing(
            localPlayer, 1, ActorType::Player, AllowSort_true);
        if (being)
        {
            if (abs(being->getTileX() - localPlayer->getTileX()) > 1
                || abs(being->getTileY() - localPlayer->getTileY()) > 1)
            {
                being = nullptr;
            }
        }
    }
    return being;
}

static Item *getItemByInvIndex(const InputEvent &event,
                               const InventoryTypeT invType)
{
    const int index = atoi(event.args.c_str());
    const Inventory *inv = nullptr;
    switch (invType)
    {
        case InventoryType::Storage:
            inv = PlayerInfo::getStorageInventory();
            break;

        case InventoryType::Inventory:
            inv = PlayerInfo::getInventory();
            break;
        case InventoryType::Trade:
        case InventoryType::Npc:
#ifdef EATHENA_SUPPORT
        case InventoryType::Cart:
        case InventoryType::Vending:
        case InventoryType::Mail:
        case InventoryType::Craft:
#endif
        case InventoryType::TypeEnd:
        default:
            break;
    }
    if (inv)
        return inv->getItem(index);
    return nullptr;
}

static int getAmountFromEvent(const InputEvent &event,
                              Item *&item0,
                              const InventoryTypeT invType)
{
    Item *const item = getItemByInvIndex(event, invType);
    item0 = item;
    if (!item)
        return 0;

    std::string str = event.args;
    removeToken(str, " ");

    if (str.empty())
        return 0;

    int amount = 0;
    if (str[0] == '-')
    {
        if (str.size() > 1)
        {
            amount = item->getQuantity() - atoi(str.substr(1).c_str());
            if (amount <= 0 || amount > item->getQuantity())
                amount = item->getQuantity();
        }
    }
    else if (str == "/")
    {
        amount = item->getQuantity() / 2;
    }
    else if (str == "all")
    {
        amount = item->getQuantity();
    }
    else
    {
        amount = atoi(str.c_str());
    }
    return amount;
}

impHandler(emote)
{
    const int emotion = 1 + (event.action - InputAction::EMOTE_1);
    if (emotion > 0)
    {
        if (emoteShortcut)
            emoteShortcut->useEmotePlayer(emotion);
        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }

    return false;
}

impHandler(outfit)
{
    if (inputManager.isActionActive(InputAction::WEAR_OUTFIT))
    {
        const int num = event.action - InputAction::OUTFIT_1;
        if (outfitWindow && num >= 0)
        {
            outfitWindow->wearOutfit(num);
            if (Game::instance())
                Game::instance()->setValidSpeed();
            return true;
        }
    }
    else if (inputManager.isActionActive(InputAction::COPY_OUTFIT))
    {
        const int num = event.action - InputAction::OUTFIT_1;
        if (outfitWindow && num >= 0)
        {
            outfitWindow->copyOutfit(num);
            if (Game::instance())
                Game::instance()->setValidSpeed();
            return true;
        }
    }

    return false;
}

impHandler0(mouseClick)
{
    if (!guiInput || !gui)
        return false;

    int mouseX, mouseY;
    Gui::getMouseState(mouseX, mouseY);
    guiInput->simulateMouseClick(mouseX, mouseY, MouseButton::RIGHT);
    return true;
}

impHandler0(ok)
{
    // Close the Browser if opened
    if (helpWindow && helpWindow->isWindowVisible())
    {
        helpWindow->setVisible(Visible_false);
        return true;
    }
    // Close the config window, cancelling changes if opened
    else if (setupWindow && setupWindow->isWindowVisible())
    {
        setupWindow->action(ActionEvent(nullptr, "cancel"));
        return true;
    }
    else if (NpcDialog *const dialog = NpcDialog::getActive())
    {
        dialog->action(ActionEvent(nullptr, "ok"));
        return true;
    }
    else if (popupMenu->isPopupVisible())
    {
        popupMenu->select();
    }
    return false;
}

impHandler(shortcut)
{
    if (itemShortcutWindow)
    {
        const int num = itemShortcutWindow->getTabIndex();
        if (num >= 0 && num < CAST_S32(SHORTCUT_TABS))
        {
            if (itemShortcut[num])
            {
                itemShortcut[num]->useItem(event.action
                    - InputAction::SHORTCUT_1);
            }
        }
        return true;
    }
    return false;
}

impHandler0(quit)
{
    if (!Game::instance())
        return false;
    if (popupManager && popupManager->isPopupMenuVisible())
    {
        popupManager->closePopupMenu();
        return true;
    }
    else if (!quitDialog)
    {
        CREATEWIDGETV(quitDialog, QuitDialog,
            &quitDialog);
        quitDialog->requestMoveToTop();
        return true;
    }
    return false;
}

impHandler0(dropItem0)
{
    if (dropShortcut)
    {
        dropShortcut->dropFirst();
        return true;
    }
    return false;
}

impHandler0(dropItem)
{
    if (dropShortcut)
    {
        dropShortcut->dropItems();
        return true;
    }
    return false;
}

impHandler(dropItemId)
{
    const Inventory *const inv = PlayerInfo::getInventory();
    if (!inv)
        return false;

    // +++ ignoring item color for now
    Item *const item = inv->findItem(atoi(event.args.c_str()),
        ItemColor_one);

    if (item && !PlayerInfo::isItemProtected(item->getId()))
    {
        ItemAmountWindow::showWindow(ItemAmountWindowUsage::ItemDrop,
            inventoryWindow, item);
    }
    return true;
}

impHandler(dropItemInv)
{
    Item *const item = getItemByInvIndex(event, InventoryType::Inventory);
    if (item && !PlayerInfo::isItemProtected(item->getId()))
    {
        ItemAmountWindow::showWindow(ItemAmountWindowUsage::ItemDrop,
            inventoryWindow, item);
    }
    return true;
}

impHandler(dropItemIdAll)
{
    const Inventory *const inv = PlayerInfo::getInventory();
    if (!inv)
        return false;

    // +++ ignoring item color for now
    Item *const item = inv->findItem(atoi(event.args.c_str()),
        ItemColor_one);

    if (item && !PlayerInfo::isItemProtected(item->getId()))
        PlayerInfo::dropItem(item, item->getQuantity(), Sfx_true);
    return true;
}

impHandler(dropItemInvAll)
{
    Item *const item = getItemByInvIndex(event, InventoryType::Inventory);
    if (item && !PlayerInfo::isItemProtected(item->getId()))
        PlayerInfo::dropItem(item, item->getQuantity(), Sfx_true);
    return true;
}

impHandler(heal)
{
#ifdef EATHENA_SUPPORT
    if (Net::getNetworkType() != ServerType::TMWATHENA)
        return false;
#endif
    if (actorManager && localPlayer)
    {
        std::string args = event.args;

        if (!args.empty())
        {
            const Being *being = nullptr;
            if (args[0] == ':')
            {
                being = actorManager->findBeing(fromInt(atoi(
                    args.substr(1).c_str()), BeingId));
                if (being && being->getType() == ActorType::Monster)
                    being = nullptr;
            }
            else
            {
                being = actorManager->findBeingByName(args, ActorType::Player);
            }
            if (being)
                actorManager->heal(being);
        }
        else
        {
            Being *target = localPlayer->getTarget();
            if (inputManager.isActionActive(InputAction::STOP_ATTACK))
            {
                if (!target || target->getType() != ActorType::Player)
                {
                    target = actorManager->findNearestLivingBeing(
                        localPlayer, 10, ActorType::Player, AllowSort_true);
                }
            }
            else
            {
                if (!target)
                    target = localPlayer;
            }
            actorManager->heal(target);
        }

        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    return false;
}

impHandler0(healmd)
{
#ifdef EATHENA_SUPPORT
    if (Net::getNetworkType() != ServerType::TMWATHENA)
        return false;
#endif
    if (actorManager)
    {
        const int matk = PlayerInfo::getStatEffective(Attributes::MATK);
        int maxHealingRadius;

        // magic levels < 2
        if (PlayerInfo::getSkillLevel(340) < 2
            || PlayerInfo::getSkillLevel(341) < 2)
        {
            maxHealingRadius = matk / 100 + 1;
        }
        else
        {
            maxHealingRadius = (12 * fastSqrtInt(matk) + matk) / 100 + 1;
        }

        Being *target = actorManager->findMostDamagedPlayer(maxHealingRadius);
        if (target)
            actorManager->heal(target);

        if (Game::instance())
            Game::instance()->setValidSpeed();
        return true;
    }
    return false;
}

impHandler0(itenplz)
{
#ifdef EATHENA_SUPPORT
    if (Net::getNetworkType() != ServerType::TMWATHENA)
        return false;
#endif
    if (actorManager)
    {
        if (playerHandler &&
            playerHandler->canUseMagic() &&
            PlayerInfo::getAttribute(Attributes::MP) >= 3)
        {
            actorManager->itenplz();
        }
        return true;
    }
    return false;
}

impHandler0(setHome)
{
    if (localPlayer)
    {
        localPlayer->setHome();
        return true;
    }
    return false;
}

impHandler0(magicAttack)
{
#ifdef EATHENA_SUPPORT
    if (Net::getNetworkType() != ServerType::TMWATHENA)
        return false;
#endif
    if (localPlayer)
    {
        localPlayer->magicAttack();
        return true;
    }
    return false;
}

impHandler0(copyEquippedToOutfit)
{
    if (outfitWindow)
    {
        outfitWindow->copyFromEquiped();
        return true;
    }
    return false;
}

impHandler(pickup)
{
    if (!localPlayer)
        return false;

    const std::string args = event.args;
    if (args.empty())
    {
        localPlayer->pickUpItems();
    }
    else
    {
        FloorItem *const item = actorManager->findItem(fromInt(
            atoi(args.c_str()), BeingId));
        if (item)
            localPlayer->pickUp(item);
    }
    return true;
}

static void doSit()
{
    if (inputManager.isActionActive(InputAction::EMOTE))
        localPlayer->updateSit();
    else
        localPlayer->toggleSit();
}

impHandler0(sit)
{
    if (localPlayer)
    {
        doSit();
        return true;
    }
    return false;
}

impHandler0(screenshot)
{
    Game::createScreenshot();
    return true;
}

impHandler0(ignoreInput)
{
    return true;
}

impHandler(buy)
{
    if (!serverFeatures)
        return false;
    const std::string args = event.args;
    Being *being = findBeing(args, false);
    if (!being && !serverFeatures->haveVending())
    {
        if (whoIsOnline)
        {
            const std::set<std::string> &players =
                whoIsOnline->getOnlineNicks();
            if (players.find(args) != players.end())
            {
                if (buySellHandler)
                    buySellHandler->requestSellList(args);
                return true;
            }
        }
        return false;
    }

    if (!being)
        being = findBeing(args, true);

    if (!being)
        return false;

    if (being->getType() == ActorType::Npc)
    {
        if (npcHandler)
            npcHandler->buy(being->getId());
        return true;
    }
    else if (being->getType() == ActorType::Player)
    {
#ifdef EATHENA_SUPPORT
        if (vendingHandler && serverFeatures->haveVending())
            vendingHandler->open(being);
        else
#endif
        if (buySellHandler)
            buySellHandler->requestSellList(being->getName());
        return true;
    }
    return false;
}

impHandler(sell)
{
    if (!serverFeatures)
        return false;

    const std::string args = event.args;
    Being *being = findBeing(args, false);
    if (!being && !serverFeatures->haveVending())
    {
        if (whoIsOnline)
        {
            const std::set<std::string> &players =
                whoIsOnline->getOnlineNicks();
            if (players.find(args) != players.end())
            {
                if (buySellHandler)
                    buySellHandler->requestBuyList(args);
                return true;
            }
        }
        return false;
    }

    if (!being)
        being = findBeing(args, true);

    if (!being)
        return false;

    if (being->getType() == ActorType::Npc)
    {
        if (npcHandler)
            npcHandler->sell(being->getId());
        return true;
    }
    else if (being->getType() == ActorType::Player)
    {
#ifdef EATHENA_SUPPORT
        if (buyingStoreHandler && serverFeatures->haveVending())
            buyingStoreHandler->open(being);
        else
#endif
        if (buySellHandler)
            buySellHandler->requestBuyList(being->getName());
        return true;
    }
    return false;
}

impHandler(talk)
{
    Being *being = findBeing(event.args, true);
    if (!being)
        return false;

    if (being->canTalk())
    {
        being->talkTo();
    }
    else if (being->getType() == ActorType::Player)
    {
        CREATEWIDGET(BuySellDialog,
            being->getName());
    }
    return true;
}

impHandler0(stopAttack)
{
    if (localPlayer)
    {
        localPlayer->stopAttack();
        // not consume if target attack key pressed
        if (inputManager.isActionActive(InputAction::TARGET_ATTACK))
            return false;
        return true;
    }
    return false;
}

impHandler0(untarget)
{
    if (localPlayer)
    {
        localPlayer->untarget();
        return true;
    }
    return false;
}

impHandler(attack)
{
    if (!localPlayer || !actorManager)
        return false;

    Being *target = nullptr;
    std::string args = event.args;
    if (!args.empty())
    {
        if (args[0] != ':')
        {
            target = actorManager->findNearestByName(args);
        }
        else
        {
            target = actorManager->findBeing(fromInt(atoi(
                args.substr(1).c_str()), BeingId));
            if (target && target->getType() != ActorType::Monster)
                target = nullptr;
        }
    }
    if (!target)
        target = localPlayer->getTarget();
    else
        localPlayer->setTarget(target);
    if (target)
        localPlayer->attack(target, true);
    return true;
}

impHandler(targetAttack)
{
    if (localPlayer && actorManager)
    {
        Being *target = nullptr;
        std::string args = event.args;
        const bool newTarget = !inputManager.isActionActive(
            InputAction::STOP_ATTACK);

        if (!args.empty())
        {
            if (args[0] != ':')
            {
                target = actorManager->findNearestByName(args);
            }
            else
            {
                target = actorManager->findBeing(fromInt(atoi(
                    args.substr(1).c_str()), BeingId));
                if (target && target->getType() != ActorType::Monster)
                    target = nullptr;
            }
        }

        if (!target && !settings.targetingType)
            target = localPlayer->getTarget();

        if (!target)
        {
            target = actorManager->findNearestLivingBeing(
                localPlayer, 90, ActorType::Monster, AllowSort_true);
        }

        localPlayer->attack2(target, newTarget);
        return true;
    }
    return false;
}

impHandler0(attackHuman)
{
    if (!actorManager || !localPlayer)
        return false;

    Being *const target = actorManager->findNearestPvpPlayer();
    if (target)
    {
        localPlayer->setTarget(target);
        localPlayer->attack2(target, true);
    }
    return true;
}

impHandler0(safeVideoMode)
{
    WindowManager::setFullScreen(false);

    return true;
}

impHandler0(stopSit)
{
    if (localPlayer)
    {
        localPlayer->stopAttack();
        // not consume if target attack key pressed
        if (inputManager.isActionActive(InputAction::TARGET_ATTACK))
            return false;
        if (!localPlayer->getTarget())
        {
            doSit();
            return true;
        }
        return true;
    }
    return false;
}

impHandler0(showKeyboard)
{
#ifdef ANDROID
#ifdef USE_SDL2
    if (SDL_IsTextInputActive())
        SDL_StopTextInput();
    else
        SDL_StartTextInput();
#else
    SDL_ANDROID_ToggleScreenKeyboardTextInput(nullptr);
#endif
    return true;
#else
    return false;
#endif
}

impHandler0(showWindows)
{
    if (popupMenu)
    {
        popupMenu->showWindowsPopup();
        return true;
    }
    return false;
}

impHandler0(openTrade)
{
    const Being *const being = localPlayer->getTarget();
    if (being && being->getType() == ActorType::Player)
    {
        if (tradeHandler)
            tradeHandler->request(being);
        tradePartnerName = being->getName();
        if (tradeWindow)
            tradeWindow->clear();
        return true;
    }
    return false;
}

impHandler0(ipcToggle)
{
    if (ipc)
    {
        IPC::stop();
        if (!ipc)
        {
            debugChatTab->chatLog("IPC service stopped.",
                ChatMsgType::BY_SERVER);
        }
        else
        {
            debugChatTab->chatLog("Unable to stop IPC service.",
                ChatMsgType::BY_SERVER);
        }
    }
    else
    {
        IPC::start();
        if (ipc)
        {
            debugChatTab->chatLog(
                strprintf("IPC service available on port %d", ipc->getPort()),
                ChatMsgType::BY_SERVER);
        }
        else
        {
            debugChatTab->chatLog("Unable to start IPC service",
                ChatMsgType::BY_SERVER);
        }
    }
    return true;
}

impHandler(where)
{
    ChatTab *const tab = event.tab != nullptr ? event.tab : debugChatTab;
    if (!tab)
        return false;
    std::ostringstream where;
    where << Game::instance()->getCurrentMapName() << ", coordinates: "
        << ((localPlayer->getPixelX() - mapTileSize / 2) / mapTileSize)
        << ", " << ((localPlayer->getPixelY() - mapTileSize) / mapTileSize);
    tab->chatLog(where.str(), ChatMsgType::BY_SERVER);
    return true;
}

impHandler0(who)
{
    if (chatHandler)
        chatHandler->who();
    return true;
}

impHandler0(cleanGraphics)
{
    resourceManager->clearCache();

    if (debugChatTab)
    {
        // TRANSLATORS: clear graphics command message
        debugChatTab->chatLog(_("Cache cleaned"),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler0(cleanFonts)
{
    if (gui)
        gui->clearFonts();
    if (debugChatTab)
    {
        // TRANSLATORS: clear fonts cache message
        debugChatTab->chatLog(_("Cache cleaned"),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler(trade)
{
    if (!actorManager)
        return false;

    const Being *being = actorManager->findBeingByName(
        event.args, ActorType::Player);
    if (!being)
        being = localPlayer->getTarget();
    if (being)
    {
        if (tradeHandler)
            tradeHandler->request(being);
        tradePartnerName = being->getName();
        if (tradeWindow)
            tradeWindow->clear();
    }
    return true;
}

impHandler0(priceLoad)
{
    if (shopWindow)
    {
        shopWindow->loadList();
        return true;
    }
    return false;
}

impHandler0(priceSave)
{
    if (shopWindow)
    {
        shopWindow->saveList();
        return true;
    }
    return false;
}

impHandler0(cacheInfo)
{
    if (!chatWindow || !debugChatTab)
        return false;

/*
    Font *const font = chatWindow->getFont();
    if (!font)
        return;

    const TextChunkList *const cache = font->getCache();
    if (!cache)
        return;

    unsigned int all = 0;
    // TRANSLATORS: chat fonts message
    debugChatTab->chatLog(_("font cache size"), ChatMsgType::BY_SERVER);
    std::string str;
    for (int f = 0; f < 256; f ++)
    {
        if (!cache[f].size)
        {
            const unsigned int sz = CAST_S32(cache[f].size);
            all += sz;
            str.append(strprintf("%d: %u, ", f, sz));
        }
    }
    debugChatTab->chatLog(str, ChatMsgType::BY_SERVER);
    // TRANSLATORS: chat fonts message
    debugChatTab->chatLog(strprintf("%s %d", _("Cache size:"), all),
        ChatMsgType::BY_SERVER);
#ifdef DEBUG_FONT_COUNTERS
    debugChatTab->chatLog("", ChatMsgType::BY_SERVER);
    debugChatTab->chatLog(strprintf("%s %d",
        // TRANSLATORS: chat fonts message
        _("Created:"), font->getCreateCounter()),
        ChatMsgType::BY_SERVER);
    debugChatTab->chatLog(strprintf("%s %d",
        // TRANSLATORS: chat fonts message
        _("Deleted:"), font->getDeleteCounter()),
        ChatMsgType::BY_SERVER);
#endif
*/
    return true;
}

impHandler0(disconnect)
{
    if (gameHandler)
        gameHandler->disconnect2();
    return true;
}

impHandler(undress)
{
    if (!actorManager || !localPlayer)
        return false;

    const std::string args = event.args;

    Being *target = nullptr;
    if (!args.empty())
    {
        if (args[0] == ':')
        {
            target = actorManager->findBeing(fromInt(atoi(
                args.substr(1).c_str()), BeingId));
            if (target && target->getType() == ActorType::Monster)
                target = nullptr;
        }
        else
        {
            target = actorManager->findNearestByName(event.args);
        }
    }

    if (!target)
        target = localPlayer->getTarget();
    if (target && beingHandler)
        beingHandler->undress(target);
    return true;
}

impHandler0(dirs)
{
    if (!debugChatTab)
        return false;

    debugChatTab->chatLog("config directory: "
        + settings.configDir,
        ChatMsgType::BY_SERVER);
    debugChatTab->chatLog("logs directory: "
        + settings.localDataDir,
        ChatMsgType::BY_SERVER);
    debugChatTab->chatLog("screenshots directory: "
        + settings.screenshotDir,
        ChatMsgType::BY_SERVER);
    debugChatTab->chatLog("temp directory: "
        + settings.tempDir,
        ChatMsgType::BY_SERVER);
    return true;
}

impHandler0(uptime)
{
    if (!debugChatTab)
        return false;

    if (cur_time < start_time)
    {
        // TRANSLATORS: uptime command
        debugChatTab->chatLog(strprintf(_("Client uptime: %s"), "unknown"),
            ChatMsgType::BY_SERVER);
    }
    else
    {
        // TRANSLATORS: uptime command
        debugChatTab->chatLog(strprintf(_("Client uptime: %s"),
            timeDiffToString(cur_time - start_time).c_str()),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

#ifdef DEBUG_DUMP_LEAKS1
static void showRes(std::string str, ResourceManager::Resources *res)
{
    if (!res)
        return;

    str.append(toString(res->size()));
    if (debugChatTab)
        debugChatTab->chatLog(str, ChatMsgType::BY_SERVER);
    logger->log(str);
    ResourceManager::ResourceIterator iter = res->begin();
    const ResourceManager::ResourceIterator iter_end = res->end();
    while (iter != iter_end)
    {
        if (iter->second && iter->second->getRefCount())
        {
            char type = ' ';
            char isNew = 'N';
            if (iter->second->getDumped())
                isNew = 'O';
            else
                iter->second->setDumped(true);

            SubImage *const subImage = dynamic_cast<SubImage *const>(
                iter->second);
            Image *const image = dynamic_cast<Image *const>(iter->second);
            int id = 0;
            if (subImage)
                type = 'S';
            else if (image)
                type = 'I';
            if (image)
                id = image->getGLImage();
            logger->log("Resource %c%c: %s (%d) id=%d", type,
                isNew, iter->second->getIdPath().c_str(),
                iter->second->getRefCount(), id);
        }
        ++ iter;
    }
}

impHandler(dump)
{
    if (!debugChatTab)
        return false;

    if (!event.args.empty())
    {
        ResourceManager::Resources *res = resourceManager->getResources();
        // TRANSLATORS: dump command
        showRes(_("Resource images:"), res);
        res = resourceManager->getOrphanedResources();
        // TRANSLATORS: dump command
        showRes(_("Resource orphaned images:"), res);
    }
    else
    {
        ResourceManager::Resources *res = resourceManager->getResources();
        // TRANSLATORS: dump command
        debugChatTab->chatLog(_("Resource images:") + toString(res->size()),
            ChatMsgType::BY_SERVER);
        res = resourceManager->getOrphanedResources();
        // TRANSLATORS: dump command
        debugChatTab->chatLog(_("Resource orphaned images:")
            + toString(res->size()),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

#elif defined ENABLE_MEM_DEBUG
impHandler0(dump)
{
    check_leaks();
    return true;
}
#else
impHandler0(dump)
{
    return true;
}
#endif

impHandler0(serverIgnoreAll)
{
    if (chatHandler)
        chatHandler->ignoreAll();
    return true;
}

impHandler0(serverUnIgnoreAll)
{
    if (chatHandler)
        chatHandler->unIgnoreAll();
    return true;
}

PRAGMA6(GCC diagnostic push)
PRAGMA6(GCC diagnostic ignored "-Wnull-dereference")
impHandler0(error)
{
    int *const ptr = nullptr;
    *(ptr + 1) = 20;
//    logger->log("test %d", *ptr);
    exit(1);
}
PRAGMA6(GCC diagnostic pop)

impHandler(dumpGraphics)
{
    std::string str = strprintf("%s,%s,%dX%dX%d,", PACKAGE_OS, SMALL_VERSION,
        mainGraphics->getWidth(), mainGraphics->getHeight(),
        mainGraphics->getBpp());

    if (mainGraphics->getFullScreen())
        str.append("F");
    else
        str.append("W");
    if (mainGraphics->getHWAccel())
        str.append("H");
    else
        str.append("S");

    if (mainGraphics->getDoubleBuffer())
        str.append("D");
    else
        str.append("_");

#if defined USE_OPENGL
    str.append(strprintf(",%d", mainGraphics->getOpenGL()));
#else
    str.append(",0");
#endif

    str.append(strprintf(",%f,", static_cast<double>(settings.guiAlpha)))
        .append(config.getBoolValue("adjustPerfomance") ? "1" : "0")
        .append(config.getBoolValue("alphaCache") ? "1" : "0")
        .append(config.getBoolValue("enableMapReduce") ? "1" : "0")
        .append(config.getBoolValue("beingopacity") ? "1" : "0")
        .append(",")
        .append(config.getBoolValue("enableAlphaFix") ? "1" : "0")
        .append(config.getBoolValue("disableAdvBeingCaching") ? "1" : "0")
        .append(config.getBoolValue("disableBeingCaching") ? "1" : "0")
        .append(config.getBoolValue("particleeffects") ? "1" : "0")
        .append(strprintf(",%d-%d", fps, config.getIntValue("fpslimit")));
    outStringNormal(event.tab, str, str);
    return true;
}

impHandler0(dumpEnvironment)
{
    logger->log1("Start environment variables");
    for (char **env = environ; *env; ++ env)
        logger->log1(*env);
    logger->log1("End environment variables");
    if (debugChatTab)
    {
        // TRANSLATORS: dump environment command
        debugChatTab->chatLog(_("Environment variables dumped"),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler(dumpTests)
{
    const std::string str = config.getStringValue("testInfo");
    outStringNormal(event.tab, str, str);
    return true;
}

impHandler0(dumpOGL)
{
#if defined(USE_OPENGL) && !defined(ANDROID) && !defined(__native_client__)
    NormalOpenGLGraphics::dumpSettings();
#endif
    return true;
}

#ifdef USE_OPENGL
impHandler(dumpGL)
{
    std::string str = graphicsManager.getGLVersion();
    outStringNormal(event.tab, str, str);
    return true;
}
#else
impHandler0(dumpGL)
{
    return true;
}
#endif

impHandler(dumpMods)
{
    std::string str = "enabled mods: " + serverConfig.getValue("mods", "");
    outStringNormal(event.tab, str, str);
    return true;
}

#if defined USE_OPENGL && defined DEBUG_SDLFONT
impHandler0(testSdlFont)
{
    Font *font = new Font("fonts/dejavusans.ttf", 18);
    timespec time1;
    timespec time2;
    NullOpenGLGraphics *nullGraphics = new NullOpenGLGraphics;
    std::vector<std::string> data;
    volatile int width = 0;

    for (int f = 0; f < 300; f ++)
        data.push_back("test " + toString(f) + "string");
    nullGraphics->beginDraw();

    clock_gettime(CLOCK_MONOTONIC, &time1);
    Color color(0, 0, 0, 255);

    for (int f = 0; f < 500; f ++)
    {
        FOR_EACH (std::vector<std::string>::const_iterator, it, data)
        {
            width += font->getWidth(*it);
            font->drawString(nullGraphics, color, color, *it, 10, 10);
        }
        FOR_EACH (std::vector<std::string>::const_iterator, it, data)
            font->drawString(nullGraphics, color, color, *it, 10, 10);

        font->doClean();
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);

    delete nullGraphics;
    delete font;

    int64_t diff = (static_cast<long long int>(
        time2.tv_sec) * 1000000000LL + static_cast<long long int>(
        time2.tv_nsec)) / 100000 - (static_cast<long long int>(
        time1.tv_sec) * 1000000000LL + static_cast<long long int>(
        time1.tv_nsec)) / 100000;
    if (debugChatTab)
    {
        debugChatTab->chatLog("sdlfont time: " + toString(diff),
            ChatMsgType::BY_SERVER);
    }
    return true;
}
#endif

impHandler0(createItems)
{
    BuyDialog *const dialog = CREATEWIDGETR0(BuyDialog);
    const ItemDB::ItemInfos &items = ItemDB::getItemInfos();
    FOR_EACH (ItemDB::ItemInfos::const_iterator, it, items)
    {
        const ItemInfo *const info = (*it).second;
        if (!info)
            continue;
        const int id = info->getId();
        if (id <= 500)
            continue;

        dialog->addItem(id,
            ItemType::Unknown,
            ItemColor_one,
            100,
            0);
    }
    dialog->sort();
    return true;
}

impHandler(uploadConfig)
{
    // TRANSLATORS: upload config chat message
    uploadFile(_("Uploaded config into:"),
        config.getFileName(),
        "?xml",
        event.tab);
    return true;
}

impHandler(uploadServerConfig)
{
    // TRANSLATORS: upload config chat message
    uploadFile(_("Uploaded server config into:"),
        serverConfig.getFileName(),
        "?xml",
        event.tab);
    return true;
}

impHandler(uploadLog)
{
    // TRANSLATORS: upload log chat message
    uploadFile(_("Uploaded log into:"),
        settings.logFileName,
        "?txt",
        event.tab);
    return true;
}

impHandler0(mercenaryFire)
{
#ifdef EATHENA_SUPPORT
    if (mercenaryHandler)
        mercenaryHandler->fire();
    return true;
#else
    return false;
#endif
}

impHandler(useItem)
{
    const int itemId = atoi(event.args.c_str());

    if (itemId < SPELL_MIN_ID)
    {
        const Inventory *const inv = PlayerInfo::getInventory();
        if (inv)
        {
            // +++ ignoring item color for now
            const Item *const item = inv->findItem(itemId,
                ItemColor_one);
            PlayerInfo::useEquipItem(item, Sfx_true);
        }
    }
    else if (itemId < SKILL_MIN_ID && spellManager)
    {
        spellManager->useItem(itemId);
    }
    else if (skillDialog)
    {
        skillDialog->useItem(itemId,
            fromBool(config.getBoolValue("skillAutotarget"), AutoTarget),
            0);
    }
    return true;
}

impHandler(useItemInv)
{
    Item *const item = getItemByInvIndex(event, InventoryType::Inventory);
    PlayerInfo::useEquipItem(item, Sfx_true);
    return true;
}

impHandler(invToStorage)
{
    Item *item = nullptr;
    const int amount = getAmountFromEvent(event, item,
        InventoryType::Inventory);
    if (!item)
        return true;
    if (amount)
    {
        if (inventoryHandler)
        {
            inventoryHandler->moveItem2(InventoryType::Inventory,
                item->getInvIndex(),
                amount,
                InventoryType::Storage);
        }
    }
    else
    {
        ItemAmountWindow::showWindow(ItemAmountWindowUsage::StoreAdd,
            inventoryWindow, item);
    }
    return true;
}

impHandler(tradeAdd)
{
    Item *item = nullptr;
    const int amount = getAmountFromEvent(event, item,
        InventoryType::Inventory);
    if (!item || PlayerInfo::isItemProtected(item->getId()))
        return true;

    if (amount)
    {
        if (tradeWindow)
            tradeWindow->tradeItem(item, amount, true);
    }
    else
    {
        ItemAmountWindow::showWindow(ItemAmountWindowUsage::TradeAdd,
            tradeWindow, item);
    }
    return true;
}

impHandler(storageToInv)
{
    Item *item = nullptr;
    const int amount = getAmountFromEvent(event, item, InventoryType::Storage);
    if (amount)
    {
        if (inventoryHandler && item)
        {
            inventoryHandler->moveItem2(InventoryType::Storage,
                item->getInvIndex(),
                amount,
                InventoryType::Inventory);
        }
    }
    else
    {
        ItemAmountWindow::showWindow(ItemAmountWindowUsage::StoreRemove,
            storageWindow, item);
    }
    return true;
}

impHandler(protectItem)
{
    const int id = atoi(event.args.c_str());
    if (id > 0)
        PlayerInfo::protectItem(id);
    return true;
}

impHandler(unprotectItem)
{
    const int id = atoi(event.args.c_str());
    if (id > 0)
        PlayerInfo::unprotectItem(id);
    return true;
}

impHandler(kick)
{
    if (!localPlayer || !actorManager)
        return false;

    Being *target = nullptr;
    std::string args = event.args;
    if (!args.empty())
    {
        if (args[0] != ':')
        {
            target = actorManager->findNearestByName(args);
        }
        else
        {
            target = actorManager->findBeing(fromInt(atoi(
                args.substr(1).c_str()), BeingId));
        }
    }
    if (!target)
        target = localPlayer->getTarget();
    if (target && adminHandler)
        adminHandler->kick(target->getId());
    return true;
}

impHandler0(clearDrop)
{
    if (dropShortcut)
        dropShortcut->clear();
    return true;
}

impHandler0(testInfo)
{
    if (actorManager)
    {
        logger->log("actors count: %d", CAST_S32(
            actorManager->size()));
        return true;
    }
    return false;
}

impHandlerE(craftKey)
{
#ifdef EATHENA_SUPPORT
    const int slot = (event.action - InputAction::CRAFT_1);
    if (slot >= 0 && slot < 9)
    {
        if (inventoryWindow)
            inventoryWindow->moveItemToCraft(slot);
        return true;
    }
#endif
    return false;
}

impHandler0(resetGameModifiers)
{
    GameModifiers::resetModifiers();
    return true;
}

impHandler(barToChat)
{
    if (chatWindow)
    {
        chatWindow->addInputText(event.args);
        return true;
    }
    return false;
}

impHandler(seen)
{
    if (!actorManager)
        return false;

    ChatTab *tab = event.tab;
    if (!tab)
        tab = localChatTab;
    if (!tab)
        return false;

    if (config.getBoolValue("enableIdCollecting") == false)
    {
        // TRANSLATORS: last seen disabled warning
        tab->chatLog(_("Last seen disabled. "
            "Enable in players / collect players id and seen log."),
            ChatMsgType::BY_SERVER);
        return true;
    }

    const std::string name = event.args;
    if (name.empty())
        return false;

    std::string dir = settings.usersDir;
    dir.append(stringToHexPath(name)).append("/seen.txt");
    if (Files::existsLocal(dir))
    {
        StringVect lines;
        Files::loadTextFileLocal(dir, lines);
        if (lines.size() < 3)
        {
            // TRANSLATORS: last seen error
            tab->chatLog(_("You not saw this nick."),
                ChatMsgType::BY_SERVER);
            return true;
        }
        const std::string message = strprintf(
            // TRANSLATORS: last seen message
            _("Last seen for %s: %s"),
            name.c_str(),
            lines[2].c_str());
        tab->chatLog(message, ChatMsgType::BY_SERVER);
    }
    else
    {
        // TRANSLATORS: last seen error
        tab->chatLog(_("You not saw this nick."),
            ChatMsgType::BY_SERVER);
    }

    return true;
}

impHandler(dumpMemoryUsage)
{
    if (event.tab)
        memoryManager.printAllMemory(event.tab);
    else
        memoryManager.printAllMemory(localChatTab);
    return true;
}

impHandler(setEmoteType)
{
    const std::string &args = event.args;
    if (args == "player" || args.empty())
    {
        settings.emoteType = EmoteType::Player;
    }
    else if (args == "pet")
    {
        settings.emoteType = EmoteType::Pet;
    }
#ifdef EATHENA_SUPPORT
    else if (args == "homun" || args == "homunculus")
    {
        settings.emoteType = EmoteType::Homunculus;
    }
    else if (args == "merc" || args == "mercenary")
    {
        settings.emoteType = EmoteType::Mercenary;
    }
#endif
    return true;
}

}  // namespace Actions
