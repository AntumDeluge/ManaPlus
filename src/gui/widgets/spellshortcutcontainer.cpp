/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
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

#include "gui/widgets/spellshortcutcontainer.h"

#include "dragdrop.h"
#include "itemshortcut.h"
#include "settings.h"
#include "spellshortcut.h"

#include "gui/viewport.h"

#include "gui/fonts/font.h"

#include "gui/popups/popupmenu.h"
#include "gui/popups/spellpopup.h"

#include "gui/windows/shortcutwindow.h"

#include "resources/image.h"

#include "debug.h"

SpellShortcutContainer::SpellShortcutContainer(Widget2 *const widget,
                                               const unsigned number) :
    ShortcutContainer(widget),
    mNumber(number),
    mSpellClicked(false)
{
    if (spellShortcut)
        mMaxItems = spellShortcut->getSpellsCount();
    else
        mMaxItems = 0;
}

SpellShortcutContainer::~SpellShortcutContainer()
{
}

void SpellShortcutContainer::setWidget2(const Widget2 *const widget)
{
    Widget2::setWidget2(widget);
    mForegroundColor = getThemeColor(Theme::TEXT);
    mForegroundColor2 = getThemeColor(Theme::TEXT_OUTLINE);
}

void SpellShortcutContainer::draw(Graphics *graphics)
{
    if (!spellShortcut)
        return;

    BLOCK_START("SpellShortcutContainer::draw")
    if (settings.guiAlpha != mAlpha)
    {
        mAlpha = settings.guiAlpha;
        if (mBackgroundImg)
            mBackgroundImg->setAlpha(mAlpha);
    }

    Font *const font = getFont();

    const int selectedId = spellShortcut->getSelectedItem();
    graphics->setColorAll(mForegroundColor, mForegroundColor2);
    drawBackground(graphics);

    // +++ in future need reorder images and string drawing.
    for (unsigned i = 0; i < mMaxItems; i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;

        const int itemId = getItemByIndex(i);
        if (selectedId >= 0 && itemId == selectedId)
        {
            graphics->drawRectangle(Rect(itemX + 1, itemY + 1,
                mBoxWidth - 1, mBoxHeight - 1));
        }

        if (!spellManager)
            continue;

        const TextCommand *const spell = spellManager->getSpell(itemId);
        if (spell)
        {
            if (!spell->isEmpty())
            {
                Image *const image = spell->getImage();

                if (image)
                {
                    image->setAlpha(1.0F);
                    graphics->drawImage(image, itemX, itemY);
                }
            }

            font->drawString(graphics, spell->getSymbol(),
                itemX + 2, itemY + mBoxHeight / 2);
        }
    }

    BLOCK_END("SpellShortcutContainer::draw")
}

void SpellShortcutContainer::mouseDragged(MouseEvent &event)
{
    if (event.getButton() == MouseButton::LEFT)
    {
        if (dragDrop.isEmpty() && mSpellClicked)
        {
            mSpellClicked = false;
            const int index = getIndexFromGrid(event.getX(), event.getY());

            if (index == -1)
                return;

            const int itemId = getItemByIndex(index);
            if (itemId < 0)
                return;
            event.consume();
            TextCommand *const spell = spellManager->getSpell(itemId);
            if (spell)
            {
                dragDrop.dragCommand(spell, DRAGDROP_SOURCE_SPELLS, index);
                dragDrop.setItem(spell->getId() + SPELL_MIN_ID);
            }
            else
            {
                dragDrop.clear();
                mSpellClicked = false;
            }
        }
    }
}

void SpellShortcutContainer::mousePressed(MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
        return;

    const MouseButton::Type eventButton = event.getButton();
    if (eventButton == MouseButton::LEFT)
    {
        const int itemId = getItemByIndex(index);
        if (itemId > 0)
            mSpellClicked = true;
        event.consume();
    }
    else if (eventButton == MouseButton::RIGHT)
    {
    }
    else if (eventButton == MouseButton::MIDDLE)
    {
        if (!spellShortcut || !spellManager)
            return;

        event.consume();
        const int itemId = getItemByIndex(index);
        spellManager->invoke(itemId);
    }
}

void SpellShortcutContainer::mouseReleased(MouseEvent &event)
{
    if (!spellShortcut || !spellManager)
        return;

    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
    {
        dragDrop.clear();
        return;
    }

    const int itemId = getItemByIndex(index);
    const MouseButton::Type eventButton = event.getButton();

    if (eventButton == MouseButton::LEFT)
    {
        mSpellClicked = false;

        if (itemId < 0)
            return;

        const int selectedId = spellShortcut->getSelectedItem();
        event.consume();

        if (!dragDrop.isEmpty())
        {
            if (dragDrop.getSource() == DRAGDROP_SOURCE_SPELLS)
            {
                const int oldIndex = dragDrop.getTag();
                const int idx = mNumber * SPELL_SHORTCUT_ITEMS;
                spellManager->swap(idx + index, idx + oldIndex);
                spellManager->save();
                dragDrop.clear();
                dragDrop.deselect();
            }
        }
        else
        {
            if (selectedId != itemId)
            {
                const TextCommand *const
                    spell = spellManager->getSpell(itemId);
                if (spell && !spell->isEmpty())
                {
                    const int num = itemShortcutWindow->getTabIndex();
                    if (num >= 0 && num < static_cast<int>(SHORTCUT_TABS)
                        && itemShortcut[num])
                    {
                        itemShortcut[num]->setItemSelected(
                            spell->getId() + SPELL_MIN_ID);
                    }
                    spellShortcut->setItemSelected(spell->getId());
                }
            }
            else
            {
                const int num = itemShortcutWindow->getTabIndex();
                if (num >= 0 && num < static_cast<int>(SHORTCUT_TABS)
                    && itemShortcut[num])
                {
                    itemShortcut[num]->setItemSelected(-1);
                }
                spellShortcut->setItemSelected(-1);
            }
        }
    }
    else if (eventButton == MouseButton::RIGHT)
    {
        TextCommand *spell = nullptr;
        if (itemId >= 0)
            spell = spellManager->getSpell(itemId);

        if (spell && popupMenu)
        {
            popupMenu->showSpellPopup(viewport->mMouseX,
                viewport->mMouseY,
                spell);
        }
    }
}

// Show ItemTooltip
void SpellShortcutContainer::mouseMoved(MouseEvent &event)
{
    if (!spellPopup || !spellShortcut || !spellManager)
        return;

    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
        return;

    const int itemId = getItemByIndex(index);
    spellPopup->setVisible(false);
    const TextCommand *const spell = spellManager->getSpell(itemId);
    if (spell && !spell->isEmpty())
    {
        spellPopup->setItem(spell);
        spellPopup->view(viewport->mMouseX, viewport->mMouseY);
    }
    else
    {
        spellPopup->setVisible(false);
    }
}

void SpellShortcutContainer::mouseExited(MouseEvent &event A_UNUSED)
{
    if (spellPopup)
        spellPopup->setVisible(false);
}

void SpellShortcutContainer::widgetHidden(const Event &event A_UNUSED)
{
    if (spellPopup)
        spellPopup->setVisible(false);
}

int SpellShortcutContainer::getItemByIndex(const int index) const
{
    return spellShortcut->getItem(
        (mNumber * SPELL_SHORTCUT_ITEMS) + index);
}
