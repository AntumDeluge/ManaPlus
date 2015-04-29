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

#include "gui/windows/mailedit.h"

#include "inventory.h"
#include "item.h"

#include "being/playerinfo.h"
#include "net/mailhandler.h"

#include "gui/windows/inventorywindow.h"
#include "gui/windows/itemamountwindow.h"
#include "gui/windows/setupwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/containerplacer.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/itemcontainer.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouttype.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"

#include "utils/delete2.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "debug.h"

MailEdit *mailEditWindow = nullptr;

MailEdit::MailEdit() :
    // TRANSLATORS: mail edit window name
    Window(_("Edit mail"), false, nullptr, "mailedit.xml"),
    ActionListener(),
    // TRANSLATORS: mail edit window button
    mSendButton(new Button(this, _("Send"), "send", this)),
    // TRANSLATORS: mail edit window button
    mCloseButton(new Button(this, _("Close"), "close", this)),
    // TRANSLATORS: mail edit window button
    mAddButton(new Button(this, _("Add"), "add", this)),
    // TRANSLATORS: mail edit window label
    mToLabel(new Label(this, _("To:"))),
    // TRANSLATORS: mail edit window label
    mSubjectLabel(new Label(this, _("Subject:"))),
    // TRANSLATORS: mail edit window label
    mMoneyLabel(new Label(this, _("Money:"))),
    // TRANSLATORS: mail edit window label
    mItemLabel(new Label(this, _("Item:"))),
    // TRANSLATORS: mail edit window label
    mMessageLabel(new Label(this, _("Message:"))),
    mToField(new TextField(this)),
    mSubjectField(new TextField(this)),
    mMoneyField(new IntTextField(this, 0, 0, 10000000)),
    mMessageField(new TextField(this)),
    mInventory(new Inventory(InventoryType::MAIL, 1)),
    mItemContainer(new ItemContainer(this, mInventory)),
    mItemScrollArea(new ScrollArea(this, mItemContainer,
        getOptionBool("showitemsbackground"), "mailedit_listbackground.xml"))
{
    setWindowName("MailEdit");
    setCloseButton(true);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(false);
    setStickyButtonLock(true);
    setVisible(true);

    setDefaultSize(380, 150, ImageRect::CENTER);
    setMinWidth(200);
    setMinHeight(100);
    center();

    ContainerPlacer placer;
    placer = getPlacer(0, 0);

    mToField->setWidth(100);
    mSubjectField->setWidth(100);
    mMessageField->setWidth(100);
    mItemScrollArea->setHeight(70);

    mItemScrollArea->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mItemScrollArea->setVerticalScrollPolicy(ScrollArea::SHOW_NEVER);

    placer(0, 0, mToLabel);
    placer(1, 0, mToField, 3);
    placer(0, 1, mSubjectLabel);
    placer(1, 1, mSubjectField, 3);
    placer(0, 2, mMoneyLabel);
    placer(1, 2, mMoneyField, 3);
    placer(0, 3, mItemLabel);
    placer(1, 3, mItemScrollArea, 2, 2);
    placer(3, 4, mAddButton, 1);

    placer(0, 5, mMessageLabel);
    placer(1, 5, mMessageField, 3);
    placer(0, 6, mSendButton);
    placer(3, 6, mCloseButton);

    loadWindowState();
    enableVisibleSound(true);
}

MailEdit::~MailEdit()
{
    mailEditWindow = nullptr;
    delete2(mInventory);
}

void MailEdit::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();
    if (eventId == "close")
    {
        scheduleDelete();
    }
    else if (eventId == "send")
    {
        const int money = mMoneyField->getValue();
        if (money)
            mailHandler->setAttachMoney(money);
        const Item *const tempItem = mInventory->getItem(0);
        if (tempItem)
        {
            const Inventory *const inv = PlayerInfo::getInventory();
            const Item *const item = inv->findItem(tempItem->getId(), 1);
            if (item)
            {
                mailHandler->setAttach(item->getInvIndex(),
                    tempItem->getQuantity());
            }
        }

        mailHandler->send(mToField->getText(),
            mSubjectField->getText(),
            mMessageField->getText());
    }
    else if (eventId == "add")
    {
        Item *const item = inventoryWindow->getSelectedItem();

        if (!item)
            return;

        ItemAmountWindow::showWindow(ItemAmountWindow::MailAdd,
            this, item);
    }
}

void MailEdit::addItem(const Item *const item, const int amount)
{
    mInventory->addItem(item->getId(),
        item->getType(),
        amount,
        item->getRefine(),
        item->getColor(),
        item->getIdentified(),
        item->getDamaged(),
        item->getFavorite(),
        Equipm_false,
        Equipped_false);
}
