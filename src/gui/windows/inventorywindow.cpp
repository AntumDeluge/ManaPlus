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

#include "gui/windows/inventorywindow.h"

#include "configuration.h"
#include "item.h"
#include "units.h"

#include "being/playerinfo.h"

#include "enums/being/attributes.h"

#include "input/inputmanager.h"

#include "gui/gui.h"

#include "gui/fonts/font.h"

#include "gui/models/sortlistmodelinv.h"

#include "gui/popups/itempopup.h"
#include "gui/popups/popupmenu.h"
#include "gui/popups/textpopup.h"

#include "gui/windows/itemamountwindow.h"
#include "gui/windows/setupwindow.h"
#include "gui/windows/tradewindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/containerplacer.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/itemcontainer.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouttype.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/tabstrip.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/windowcontainer.h"

#include "net/inventoryhandler.h"

#include "resources/iteminfo.h"

#include "utils/delete2.h"

#include "debug.h"

InventoryWindow *inventoryWindow = nullptr;
InventoryWindow *storageWindow = nullptr;
#ifdef EATHENA_SUPPORT
InventoryWindow *cartWindow = nullptr;
#endif
InventoryWindow::WindowList InventoryWindow::invInstances;

InventoryWindow::InventoryWindow(Inventory *const inventory) :
    Window("Inventory", Modal_false, nullptr, "inventory.xml"),
    ActionListener(),
    KeyListener(),
    SelectionListener(),
    InventoryListener(),
    AttributeListener(),
    mInventory(inventory),
    mItems(new ItemContainer(this, mInventory)),
    mUseButton(nullptr),
    mDropButton(nullptr),
    mSplitButton(nullptr),
    mOutfitButton(nullptr),
    mShopButton(nullptr),
    mCartButton(nullptr),
    mEquipmentButton(nullptr),
    mStoreButton(nullptr),
    mRetrieveButton(nullptr),
    mInvCloseButton(nullptr),
    mWeightBar(nullptr),
    mSlotsBar(new ProgressBar(this, 0.0F, 100, 0, Theme::PROG_INVY_SLOTS,
              "slotsprogressbar.xml", "slotsprogressbar_fill.xml")),
    mFilter(nullptr),
    mSortModel(new SortListModelInv),
    mSortDropDown(new DropDown(this, mSortModel, false,
        Modal_false, this, "sort")),
    mNameFilter(new TextField(this, "", true, this, "namefilter", true)),
    mSortDropDownCell(nullptr),
    mNameFilterCell(nullptr),
    mFilterCell(nullptr),
    mSlotsBarCell(nullptr),
    mSplit(false),
    mCompactMode(false)
{
    mSlotsBar->setColor(getThemeColor(Theme::SLOTS_BAR),
        getThemeColor(Theme::SLOTS_BAR_OUTLINE));

    if (inventory)
    {
        setCaption(gettext(inventory->getName().c_str()));
        setWindowName(inventory->getName());
        switch (inventory->getType())
        {
            case InventoryType::INVENTORY:
            case InventoryType::TRADE:
            case InventoryType::NPC:
#ifdef EATHENA_SUPPORT
            case InventoryType::VENDING:
            case InventoryType::MAIL:
#endif
            case InventoryType::TYPE_END:
            default:
                mSortDropDown->setSelected(config.getIntValue(
                    "inventorySortOrder"));
                break;
            case InventoryType::STORAGE:
                mSortDropDown->setSelected(config.getIntValue(
                    "storageSortOrder"));
                break;
#ifdef EATHENA_SUPPORT
            case InventoryType::CART:
                mSortDropDown->setSelected(config.getIntValue(
                    "cartSortOrder"));
                break;
#endif
        };
    }
    else
    {
        // TRANSLATORS: inventory window name
        setCaption(_("Inventory"));
        setWindowName("Inventory");
        mSortDropDown->setSelected(0);
    }

    if (setupWindow &&
        inventory &&
        inventory->getType() != InventoryType::STORAGE)
    {
        setupWindow->registerWindowForReset(this);
    }

    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setStickyButtonLock(true);

    if (mainGraphics->mWidth > 600)
        setDefaultSize(450, 310, ImageRect::CENTER);
    else
        setDefaultSize(387, 307, ImageRect::CENTER);
    setMinWidth(310);
    setMinHeight(179);
    addKeyListener(this);

    mItems->addSelectionListener(this);

    ScrollArea *const invenScroll = new ScrollArea(this, mItems,
        getOptionBool("showbackground"), "inventory_background.xml");
    invenScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);

    const int size = config.getIntValue("fontSize");
    mFilter = new TabStrip(this, "filter_" + getWindowName(), size + 16);
    mFilter->addActionListener(this);
    mFilter->setActionEventId("tag_");

    StringVect tags = ItemDB::getTags();
    const size_t sz = tags.size();
    for (size_t f = 0; f < sz; f ++)
        mFilter->addButton(tags[f], tags[f], false);

    switch (mInventory->getType())
    {
        case InventoryType::INVENTORY:
        {
            // TRANSLATORS: inventory button
            const std::string equip = _("Equip");
            // TRANSLATORS: inventory button
            const std::string use = _("Use");
            // TRANSLATORS: inventory button
            const std::string unequip = _("Unequip");

            std::string longestUseString = getFont()->getWidth(equip) >
                getFont()->getWidth(use) ? equip : use;

            if (getFont()->getWidth(longestUseString) <
                getFont()->getWidth(unequip))
            {
                longestUseString = unequip;
            }

            mUseButton = new Button(this, longestUseString, "use", this);
            // TRANSLATORS: inventory button
            mDropButton = new Button(this, _("Drop..."), "drop", this);
            // TRANSLATORS: inventory button
            mSplitButton = new Button(this, _("Split"), "split", this);
            // TRANSLATORS: inventory outfits button
            mOutfitButton = new Button(this, _("O"), "outfit", this);
            // TRANSLATORS: inventory cart button
            mCartButton = new Button(this, _("C"), "cart", this);
            // TRANSLATORS: inventory shop button
            mShopButton = new Button(this, _("S"), "shop", this);
            // TRANSLATORS: inventory equipment button
            mEquipmentButton = new Button(this, _("E"), "equipment", this);
            mWeightBar = new ProgressBar(this, 0.0F, 100, 0,
                Theme::PROG_WEIGHT,
                "weightprogressbar.xml", "weightprogressbar_fill.xml");
            mWeightBar->setColor(getThemeColor(Theme::WEIGHT_BAR),
                getThemeColor(Theme::WEIGHT_BAR_OUTLINE));

            // TRANSLATORS: outfits button tooltip
            mOutfitButton->setDescription(_("Outfits"));
            // TRANSLATORS: cart button tooltip
            mCartButton->setDescription(_("Cart"));
            // TRANSLATORS: shop button tooltip
            mShopButton->setDescription(_("Shop"));
            // TRANSLATORS: equipment button tooltip
            mEquipmentButton->setDescription(_("Equipment"));

            place(0, 0, mWeightBar, 4);
            mSlotsBarCell = &place(4, 0, mSlotsBar, 4);
            mSortDropDownCell = &place(8, 0, mSortDropDown, 3);

            mFilterCell = &place(0, 1, mFilter, 10).setPadding(3);
            mNameFilterCell = &place(8, 1, mNameFilter, 3);

            place(0, 2, invenScroll, 11).setPadding(3);
            place(0, 3, mUseButton);
            place(1, 3, mDropButton);
            place(8, 2, mSplitButton);
            ContainerPlacer placer = getPlacer(10, 3);
            placer(0, 0, mShopButton);
            placer(1, 0, mOutfitButton);
            placer(2, 0, mCartButton);
            placer(3, 0, mEquipmentButton);

            updateWeight();
            break;
        }

        case InventoryType::STORAGE:
        {
            // TRANSLATORS: storage button
            mStoreButton = new Button(this, _("Store"), "store", this);
            // TRANSLATORS: storage button
            mRetrieveButton = new Button(this, _("Retrieve"),
                "retrieve", this);
            // TRANSLATORS: storage button
            mInvCloseButton = new Button(this, _("Close"), "close", this);

            mSlotsBarCell = &place(0, 0, mSlotsBar, 6);
            mSortDropDownCell = &place(6, 0, mSortDropDown, 1);

            mFilterCell = &place(0, 1, mFilter, 7).setPadding(3);
            mNameFilterCell = &place(6, 1, mNameFilter, 1);

            place(0, 2, invenScroll, 7, 4);
            place(0, 6, mStoreButton);
            place(1, 6, mRetrieveButton);
            place(6, 6, mInvCloseButton);
            break;
        }

#ifdef EATHENA_SUPPORT
        case InventoryType::CART:
        {
            // TRANSLATORS: storage button
            mStoreButton = new Button(this, _("Store"), "store", this);
            // TRANSLATORS: storage button
            mRetrieveButton = new Button(this, _("Retrieve"),
                "retrieve", this);
            // TRANSLATORS: storage button
            mInvCloseButton = new Button(this, _("Close"), "close", this);

            mWeightBar = new ProgressBar(this, 0.0F, 100, 0,
                Theme::PROG_WEIGHT,
                "weightprogressbar.xml", "weightprogressbar_fill.xml");
            mWeightBar->setColor(getThemeColor(Theme::WEIGHT_BAR),
                getThemeColor(Theme::WEIGHT_BAR_OUTLINE));

            mSlotsBarCell = &place(3, 0, mSlotsBar, 3);
            mSortDropDownCell = &place(6, 0, mSortDropDown, 1);

            mFilterCell = &place(0, 1, mFilter, 7).setPadding(3);
            mNameFilterCell = &place(6, 1, mNameFilter, 1);

            place(0, 0, mWeightBar, 3);
            place(0, 2, invenScroll, 7, 4);
            place(0, 6, mStoreButton);
            place(1, 6, mRetrieveButton);
            place(6, 6, mInvCloseButton);
            break;
        }
#endif

        default:
        case InventoryType::TRADE:
        case InventoryType::NPC:
#ifdef EATHENA_SUPPORT
        case InventoryType::VENDING:
        case InventoryType::MAIL:
#endif
        case InventoryType::TYPE_END:
            break;
    };

    Layout &layout = getLayout();
    layout.setRowHeight(2, LayoutType::SET);

    mInventory->addInventoyListener(this);

    invInstances.push_back(this);

    if (inventory && inventory->isMainInventory())
    {
        updateDropButton();
    }
    else
    {
        if (!invInstances.empty())
            invInstances.front()->updateDropButton();
    }

    loadWindowState();
    enableVisibleSound(true);
}

void InventoryWindow::postInit()
{
    slotsChanged(mInventory);

    mItems->setSortType(mSortDropDown->getSelected());
    widgetResized(Event(nullptr));
    if (mInventory && mInventory->getType() == InventoryType::STORAGE)
        setVisible(true);
}

InventoryWindow::~InventoryWindow()
{
    invInstances.remove(this);
    if (mInventory)
        mInventory->removeInventoyListener(this);
    if (!invInstances.empty())
        invInstances.front()->updateDropButton();

    mSortDropDown->hideDrop(false);
    delete2(mSortModel);
}

void InventoryWindow::storeSortOrder() const
{
    if (mInventory)
    {
        switch (mInventory->getType())
        {
            case InventoryType::INVENTORY:
            case InventoryType::TRADE:
            case InventoryType::NPC:
#ifdef EATHENA_SUPPORT
            case InventoryType::VENDING:
            case InventoryType::MAIL:
#endif
            case InventoryType::TYPE_END:
            default:
                config.setValue("inventorySortOrder",
                    mSortDropDown->getSelected());
                break;
            case InventoryType::STORAGE:
                config.setValue("storageSortOrder",
                    mSortDropDown->getSelected());
                break;
#ifdef EATHENA_SUPPORT
            case InventoryType::CART:
                config.setValue("cartSortOrder",
                    mSortDropDown->getSelected());
                break;
#endif
        };
    }
}

void InventoryWindow::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();
    if (eventId == "outfit")
    {
        inputManager.executeAction(InputAction::WINDOW_OUTFIT);
    }
    else if (eventId == "shop")
    {
        inputManager.executeAction(InputAction::WINDOW_SHOP);
    }
    else if (eventId == "equipment")
    {
        inputManager.executeAction(InputAction::WINDOW_EQUIPMENT);
    }
    else if (eventId == "cart")
    {
        inputManager.executeAction(InputAction::WINDOW_CART);
    }
    else if (eventId == "close")
    {
        close();
    }
    else if (eventId == "store")
    {
        if (!inventoryWindow || !inventoryWindow->isWindowVisible())
            return;

        Item *const item = inventoryWindow->getSelectedItem();

        if (!item)
            return;

        if (storageWindow)
        {
            ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd,
                this, item);
        }
#ifdef EATHENA_SUPPORT
        else if (cartWindow && cartWindow->isWindowVisible())
        {
            ItemAmountWindow::showWindow(ItemAmountWindow::CartAdd,
                this, item);
        }
#endif
    }
    else if (eventId == "sort")
    {
        mItems->setSortType(mSortDropDown->getSelected());
        storeSortOrder();
        return;
    }
    else if (eventId == "namefilter")
    {
        mItems->setName(mNameFilter->getText());
        mItems->updateMatrix();
    }
    else if (!eventId.find("tag_"))
    {
        std::string tagName = event.getId().substr(4);
        mItems->setFilter(ItemDB::getTagId(tagName));
        return;
    }

    Item *const item = mItems->getSelectedItem();

    if (!item)
        return;

    if (eventId == "use")
    {
        PlayerInfo::useEquipItem(item, Sfx_true);
    }
    if (eventId == "equip")
    {
        PlayerInfo::useEquipItem2(item, Sfx_true);
    }
    else if (eventId == "drop")
    {
        if (isStorageActive())
        {
            inventoryHandler->moveItem2(InventoryType::INVENTORY,
                item->getInvIndex(), item->getQuantity(),
                InventoryType::STORAGE);
        }
#ifdef EATHENA_SUPPORT
        else if (cartWindow && cartWindow->isWindowVisible())
        {
            inventoryHandler->moveItem2(InventoryType::INVENTORY,
                item->getInvIndex(), item->getQuantity(),
                InventoryType::CART);
        }
        else
#endif
        {
            if (PlayerInfo::isItemProtected(item->getId()))
                return;

            if (inputManager.isActionActive(static_cast<int>(
                InputAction::STOP_ATTACK)))
            {
                PlayerInfo::dropItem(item, item->getQuantity(), Sfx_true);
            }
            else
            {
                ItemAmountWindow::showWindow(ItemAmountWindow::ItemDrop,
                    this, item);
            }
        }
    }
    else if (eventId == "split")
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemSplit, this, item,
            (item->getQuantity() - 1));
    }
    else if (eventId == "retrieve")
    {
        if (storageWindow)
        {
            ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove,
                this, item);
        }
#ifdef EATHENA_SUPPORT
        else if (cartWindow && cartWindow->isWindowVisible())
        {
            ItemAmountWindow::showWindow(ItemAmountWindow::CartRemove,
                this, item);
        }
#endif
    }
}

Item *InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void InventoryWindow::unselectItem()
{
    mItems->selectNone();
}

void InventoryWindow::widgetHidden(const Event &event)
{
    Window::widgetHidden(event);
    if (itemPopup)
        itemPopup->setVisible(false);
}

void InventoryWindow::mouseClicked(MouseEvent &event)
{
    Window::mouseClicked(event);

    const int clicks = event.getClickCount();

    if (clicks == 2 && gui)
        gui->resetClickCount();

    const bool mod = (isStorageActive() && inputManager.isActionActive(
        static_cast<int>(InputAction::STOP_ATTACK)));

    const bool mod2 = (tradeWindow && tradeWindow->isWindowVisible()
        && inputManager.isActionActive(static_cast<int>(
        InputAction::STOP_ATTACK)));

    if (!mod && !mod2 && event.getButton() == MouseButton::RIGHT)
    {
        Item *const item = mItems->getSelectedItem();

        if (!item)
            return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();

        if (popupMenu)
            popupMenu->showPopup(this, mx, my, item, mInventory->getType());
    }

    if (!mInventory)
        return;

    if (event.getButton() == MouseButton::LEFT
        || event.getButton() == MouseButton::RIGHT)
    {
        Item *const item = mItems->getSelectedItem();

        if (!item)
            return;

        if (mod)
        {
            if (mInventory->isMainInventory())
            {
                if (event.getButton() == MouseButton::RIGHT)
                {
                    ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd,
                        inventoryWindow, item);
                }
                else
                {
                    inventoryHandler->moveItem2(InventoryType::INVENTORY,
                        item->getInvIndex(), item->getQuantity(),
                        InventoryType::STORAGE);
                }
            }
            else
            {
                if (event.getButton() == MouseButton::RIGHT)
                {
                    ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove,
                        inventoryWindow, item);
                }
                else
                {
                    inventoryHandler->moveItem2(InventoryType::STORAGE,
                        item->getInvIndex(), item->getQuantity(),
                        InventoryType::INVENTORY);
                }
            }
        }
        else if (mod2 && mInventory->isMainInventory())
        {
            if (PlayerInfo::isItemProtected(item->getId()))
                return;
            if (event.getButton() == MouseButton::RIGHT)
            {
                ItemAmountWindow::showWindow(ItemAmountWindow::TradeAdd,
                    tradeWindow, item);
            }
            else
            {
                if (tradeWindow)
                    tradeWindow->tradeItem(item, item->getQuantity(), true);
            }
        }
        else if (clicks == 2)
        {
            if (mInventory->isMainInventory())
            {
                if (isStorageActive())
                {
                    ItemAmountWindow::showWindow(ItemAmountWindow::StoreAdd,
                        inventoryWindow, item);
                }
                else if (tradeWindow && tradeWindow->isWindowVisible())
                {
                    if (PlayerInfo::isItemProtected(item->getId()))
                        return;
                    ItemAmountWindow::showWindow(ItemAmountWindow::TradeAdd,
                        tradeWindow, item);
                }
                else
                {
                    PlayerInfo::useEquipItem(item, Sfx_true);
                }
            }
            else
            {
                if (isStorageActive())
                {
                    ItemAmountWindow::showWindow(ItemAmountWindow::StoreRemove,
                        inventoryWindow, item);
                }
            }
        }
    }
}

void InventoryWindow::mouseMoved(MouseEvent &event)
{
    Window::mouseMoved(event);
    if (!textPopup)
        return;

    const Widget *const src = event.getSource();
    if (!src)
    {
        textPopup->hide();
        return;
    }
    const int x = event.getX();
    const int y = event.getY();
    const Rect &rect = mDimension;
    if (src == mSlotsBar || src == mWeightBar)
    {
        textPopup->show(rect.x + x, rect.y + y, strprintf(_("Money: %s"),
            Units::formatCurrency(PlayerInfo::getAttribute(
            Attributes::MONEY)).c_str()));
    }
    else
    {
        const Button *const btn = dynamic_cast<const Button *const>(src);
        if (!btn)
        {
            textPopup->hide();
            return;
        }
        const std::string text = btn->getDescription();
        if (!text.empty())
            textPopup->show(x + rect.x, y + rect.y, text);
    }
}

void InventoryWindow::mouseExited(MouseEvent &event A_UNUSED)
{
    textPopup->hide();
}

void InventoryWindow::keyPressed(KeyEvent &event)
{
    if (event.getActionId() == static_cast<int>(InputAction::GUI_MOD))
        mSplit = true;
}

void InventoryWindow::keyReleased(KeyEvent &event)
{
    if (event.getActionId() == static_cast<int>(InputAction::GUI_MOD))
        mSplit = false;
}

void InventoryWindow::valueChanged(const SelectionEvent &event A_UNUSED)
{
    if (!mInventory || !mInventory->isMainInventory())
        return;

    Item *const item = mItems->getSelectedItem();

    if (mSplit && item && inventoryHandler->
        canSplit(mItems->getSelectedItem()))
    {
        ItemAmountWindow::showWindow(ItemAmountWindow::ItemSplit,
            this, item, item->getQuantity() - 1);
    }
    updateButtons(item);
}

void InventoryWindow::updateButtons(const Item *item)
{
    if (!mInventory || !mInventory->isMainInventory())
        return;

    const Item *const selectedItem = mItems->getSelectedItem();
    if (item && selectedItem != item)
        return;

    if (!item)
        item = selectedItem;

    if (!item || item->getQuantity() == 0)
    {
        if (mUseButton)
            mUseButton->setEnabled(false);
        if (mDropButton)
            mDropButton->setEnabled(false);
        return;
    }

    if (mDropButton)
        mDropButton->setEnabled(true);

    if (mUseButton)
    {
        const ItemInfo &info = item->getInfo();
        const std::string &str = (item->isEquipment() == Equipm_true
            && item->isEquipped() == Equipped_true)
            ? info.getUseButton2() : info.getUseButton();
        if (str.empty())
        {
            mUseButton->setEnabled(false);
            mUseButton->setCaption(_("Use"));
        }
        else
        {
            mUseButton->setEnabled(true);
            mUseButton->setCaption(str);
        }
    }

    updateDropButton();

    if (mSplitButton)
    {
        if (inventoryHandler->canSplit(item))
            mSplitButton->setEnabled(true);
        else
            mSplitButton->setEnabled(false);
    }
}

void InventoryWindow::setSplitAllowed(const bool allowed)
{
    if (mSplitButton)
        mSplitButton->setVisible(allowed);
}

void InventoryWindow::close()
{
    if (!mInventory)
    {
        Window::close();
        return;
    }

    switch (mInventory->getType())
    {
        case InventoryType::INVENTORY:
#ifdef EATHENA_SUPPORT
        case InventoryType::CART:
#endif
            setVisible(false);
            break;

        case InventoryType::STORAGE:
            if (inventoryHandler)
            {
                inventoryHandler->closeStorage(InventoryType::STORAGE);
                inventoryHandler->forgotStorage();
            }
            scheduleDelete();
            break;

        default:
        case InventoryType::TRADE:
        case InventoryType::NPC:
#ifdef EATHENA_SUPPORT
        case InventoryType::VENDING:
        case InventoryType::MAIL:
#endif
        case InventoryType::TYPE_END:
            break;
    }
}

void InventoryWindow::updateWeight()
{
    if (!mInventory || !mWeightBar)
        return;
    const InventoryType::Type type = mInventory->getType();
#ifdef EATHENA_SUPPORT
    if (type != InventoryType::INVENTORY && type != InventoryType::CART)
#else
    if (type != InventoryType::INVENTORY)
#endif
    {
        return;
    }

    const bool isInv = type == InventoryType::INVENTORY;
    const int total = PlayerInfo::getAttribute(isInv
        ? Attributes::TOTAL_WEIGHT : Attributes::CART_TOTAL_WEIGHT);
    const int max = PlayerInfo::getAttribute(isInv
        ? Attributes::MAX_WEIGHT : Attributes::CART_MAX_WEIGHT);

    if (max <= 0)
        return;

    // Adjust progress bar
    mWeightBar->setProgress(static_cast<float>(total)
        / static_cast<float>(max));
    mWeightBar->setText(strprintf("%s/%s",
        Units::formatWeight(total).c_str(),
        Units::formatWeight(max).c_str()));
}

void InventoryWindow::slotsChanged(Inventory *const inventory)
{
    if (inventory == mInventory)
    {
        const int usedSlots = mInventory->getNumberOfSlotsUsed();
        const int maxSlots = mInventory->getSize();

        if (maxSlots)
        {
            mSlotsBar->setProgress(static_cast<float>(usedSlots)
                / static_cast<float>(maxSlots));
        }

        mSlotsBar->setText(strprintf("%d/%d", usedSlots, maxSlots));
        mItems->updateMatrix();
    }
}

void InventoryWindow::updateDropButton()
{
    if (!mDropButton)
        return;

#ifdef EATHENA_SUPPORT
    if (isStorageActive() || (cartWindow && cartWindow->isWindowVisible()))
#else
    if (isStorageActive())
#endif
    {
        // TRANSLATORS: inventory button
        mDropButton->setCaption(_("Store"));
    }
    else
    {
        const Item *const item = mItems->getSelectedItem();
        if (item && item->getQuantity() > 1)
        {
            // TRANSLATORS: inventory button
            mDropButton->setCaption(_("Drop..."));
        }
        else
        {
            // TRANSLATORS: inventory button
            mDropButton->setCaption(_("Drop"));
        }
    }
}

bool InventoryWindow::isInputFocused() const
{
    return mNameFilter && mNameFilter->isFocused();
}

bool InventoryWindow::isAnyInputFocused()
{
    FOR_EACH (WindowList::const_iterator, it, invInstances)
    {
        if ((*it) && (*it)->isInputFocused())
            return true;
    }
    return false;
}

InventoryWindow *InventoryWindow::getFirstVisible()
{
    std::set<Widget*> list;
    FOR_EACH (WindowList::const_iterator, it, invInstances)
    {
        if ((*it) && (*it)->isWindowVisible())
            list.insert(*it);
    }
    InventoryWindow *const window = dynamic_cast<InventoryWindow*>(
        windowContainer->findFirstWidget(list));
    return window;
}

void InventoryWindow::nextTab()
{
    const InventoryWindow *const window = getFirstVisible();
    if (window)
        window->mFilter->nextTab();
}

void InventoryWindow::prevTab()
{
    const InventoryWindow *const window = getFirstVisible();
    if (window)
        window->mFilter->prevTab();
}

void InventoryWindow::widgetResized(const Event &event)
{
    Window::widgetResized(event);

    if (!mInventory)
        return;
    const InventoryType::Type type = mInventory->getType();
#ifdef EATHENA_SUPPORT
    if (type != InventoryType::INVENTORY && type != InventoryType::CART)
#else
    if (type != InventoryType::INVENTORY)
#endif
    {
        return;
    }

    if (getWidth() < 600)
    {
        if (!mCompactMode)
        {
            mNameFilter->setVisible(false);
            mNameFilterCell->setType(LayoutCell::NONE);
            mFilterCell->setWidth(mFilterCell->getWidth() + 3);
            mCompactMode = true;
        }
    }
    else if (mCompactMode)
    {
        mNameFilter->setVisible(true);
        mNameFilterCell->setType(LayoutCell::WIDGET);
        mFilterCell->setWidth(mFilterCell->getWidth() - 3);
        mCompactMode = false;
    }
}

void InventoryWindow::setVisible(bool visible)
{
    if (!visible)
        mSortDropDown->hideDrop();
    Window::setVisible(visible);
}

void InventoryWindow::unsetInventory()
{
    if (mInventory)
    {
        mInventory->removeInventoyListener(this);
        if (mItems)
            mItems->unsetInventory();
    }
    mInventory = nullptr;
}

void InventoryWindow::attributeChanged(const int id,
                                       const int oldVal A_UNUSED,
                                       const int newVal A_UNUSED)
{
    if (id == Attributes::TOTAL_WEIGHT
        || id == Attributes::MAX_WEIGHT
        || id == Attributes::CART_TOTAL_WEIGHT
        || id == Attributes::CART_MAX_WEIGHT)
    {
        updateWeight();
    }
}
