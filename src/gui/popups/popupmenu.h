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

#ifndef GUI_POPUPS_POPUPMENU_H
#define GUI_POPUPS_POPUPMENU_H

#ifndef DYECMD

#include "const/resources/item/cards.h"

#include "enums/inventorytype.h"

#include "enums/simpletypes/beingid.h"
#include "enums/simpletypes/beingtypeid.h"
#include "enums/simpletypes/itemcolor.h"

#include "gui/widgets/linkhandler.h"
#include "gui/widgets/popup.h"

#include "listeners/playerlistener.h"
#include "listeners/renamelistener.h"

#include "localconsts.h"

class ActorSprite;
class Being;
class BrowserBox;
class Button;
class ChatTab;
class FloorItem;
class Item;
class ListModel;
class MapItem;
class ScrollArea;
class TextCommand;
class TextDialog;
class TextField;
class ProgressBar;
class Window;

struct SkillInfo;

/**
 * Window showing popup menu.
 */
class PopupMenu final : public Popup, public LinkHandler
{
    public:
        /**
         * Constructor.
         */
        PopupMenu();

        A_DELETE_COPY(PopupMenu)

        void postInit() override final;

        /**
         * Shows the being related popup menu at the specified mouse coords.
         */
        void showPopup(const int x, const int y, const Being *const being);

        /**
         * Shows the beings related popup menu at the specified mouse coords.
         */
        void showPopup(const int x, const int y,
                       const std::vector<ActorSprite*> &beings);

        void showPlayerPopup(const std::string &nick);

        /**
         * Shows the floor item related popup menu at the specified
         * mouse coords.
         */
        void showPopup(const int x, const int y,
                       const FloorItem *const floorItem);

        /**
         * Shows the related popup menu when right click on the inventory
         * at the specified mouse coordinates.
         */
        void showPopup(Window *const parent,
                       const int x, const int y,
                       const Item *const item,
                       const InventoryTypeT type);

        void showPopup(const int x, const int y, Button *const button);

        void showPopup(const int x, const int y, const ProgressBar *const bar);

        void showPopup(const int x, const int y, MapItem *const mapItem);

        void showItemPopup(const int x,
                           const int y,
                           const Item *const item);

        void showItemPopup(const int x, const int y,
                           const int itemId,
                           const ItemColor color);

        void showDropPopup(const int x,
                           const int y,
                           const Item *const item);

        void showOutfitsWindowPopup(const int x, const int y);

        void showSpellPopup(const int x, const int y, TextCommand *const cmd);

        void showAttackMonsterPopup(const int x, const int y,
                                    const std::string &name,
                                    const int type);

        void showPickupItemPopup(const int x, const int y,
                                 const std::string &name);

        void showUndressPopup(const int x, const int y,
                              const Being *const being,
                              const Item *const item);

        void showMapPopup(const int x, const int y,
                          const int x2, const int y2,
                          const bool isMinimap);

        /**
         * Shows the related popup menu when right click on the chat
         * at the specified mouse coordinates.
         */
        void showChatPopup(const int x, const int y, ChatTab *const tab);

        void showChangePos(const int x, const int y);

        void showPopup(const int x, const int y, ListModel *const model);

        void showTextFieldPopup(TextField *const input);

        void showLinkPopup(const std::string &link);

        void showWindowsPopup();

        void showNpcDialogPopup(const BeingId npcId,
                                const int x, const int y);

        void showWindowPopup(Window *const window);

        void showSkillPopup(const SkillInfo *const info);

        void showSkillLevelPopup(const SkillInfo *const info);

        void showSkillTypePopup(const SkillInfo *const info);

        void showCraftPopup();

        void showEmoteType();

        /**
         * Handles link action.
         */
        void handleLink(const std::string &link,
                        MouseEvent *event A_UNUSED) override final;

        void clear() override final;

        void moveUp();

        void moveDown();

        void select();

    private:
        void setMousePos();

        void addPlayerRelation(const std::string &name);

        void addFollow();

        void addBuySell(const Being *const being) A_NONNULL(2);

        void addParty(const std::string &nick);

        void addPartyName(const std::string &partyName);

        void addBuySellDefault();

        void addPlayerMisc();

        void addPickupFilter(const std::string &name);

        void addProtection();

        void addUseDrop(const Item *const item,
                        const bool isProtected) A_NONNULL(2);

        void addGmCommands();

        void showMuteCommands();

        void showPlayerMenu();

        void showFloorItemGMCommands();

        void showItemGMCommands();

        void addNormalRelations();

        void addWindowMenu(const Window *const window);

        void showHomunGMCommands();

        void showPetGMCommands();

        void showMercenaryGMCommands();

        void addChat(const Being *const being);

        void addMailCommands();

        void addCatchPetCommands();

        void showGMPopup(const std::string &name);

        void showPlayerGMCommands(const std::string &name);

        void showNpcGMCommands();

        void showMonsterGMCommands();

        void showAdoptCommands();

        bool addBeingMenu();

        void showSkillLevelMenu();

        BrowserBox *mBrowserBox A_NONNULLPOINTER;
        ScrollArea *mScrollArea A_NONNULLPOINTER;

        BeingId mBeingId;
        BeingId mFloorItemId;
        int mItemId;
        int mItemIndex;
        int mItemCards[maxCards];
        ItemColor mItemColor;
        MapItem *mMapItem;
        ChatTab *mTab;
        TextCommand *mSpell;
        Window *mWindow;
        RenameListener mRenameListener;
        PlayerListener mPlayerListener;
        TextDialog *mDialog;
        Button *mButton;
        std::string mName;
        std::string mExtName;
        TextField *mTextField;
        ActorTypeT mType;
        BeingTypeId mSubType;
        int mX;
        int mY;
        bool mAllowCleanMenu;

        /**
         * Shared code for the various showPopup functions.
         */
        void showPopup(int x, int y);
};

extern PopupMenu *popupMenu;

#endif  // DYE_CMD
#endif  // GUI_POPUPS_POPUPMENU_H
