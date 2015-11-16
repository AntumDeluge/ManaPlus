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

#include "gui/windows/equipmentwindow.h"

#include "configuration.h"
#include "dragdrop.h"
#include "graphicsvertexes.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "gui/fonts/font.h"

#include "gui/popups/popupmenu.h"
#include "gui/popups/itempopup.h"

#include "gui/windows/setupwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/equipmentbox.h"
#include "gui/widgets/equipmentpage.h"
#include "gui/widgets/playerbox.h"
#include "gui/widgets/tabstrip.h"

#include "resources/equipmentslots.h"
#include "resources/imageset.h"

#include "utils/delete2.h"
#include "utils/dtor.h"
#include "utils/gettext.h"

#include "net/inventoryhandler.h"

#include "debug.h"

EquipmentWindow *equipmentWindow = nullptr;
EquipmentWindow *beingEquipmentWindow = nullptr;
static const int BOX_COUNT = 27;
StringIntMap EquipmentWindow::mSlotNames;

EquipmentWindow::EquipmentWindow(Equipment *const equipment,
                                 Being *const being,
                                 const bool foring) :
    // TRANSLATORS: equipment window name
    Window(_("Equipment"), Modal_false, nullptr, "equipment.xml"),
    ActionListener(),
    mEquipment(equipment),
    mPlayerBox(new PlayerBox(this,
        "equipment_playerbox.xml",
        "equipment_selectedplayerbox.xml")),
    // TRANSLATORS: equipment window button
    mUnequip(new Button(this, _("Unequip"), "unequip", this)),
    mImageSet(nullptr),
    mBeing(being),
    mSlotBackground(),
    mSlotHighlightedBackground(),
    mVertexes(new ImageCollection),
    mPages(),
    mTabs(nullptr),
    mHighlightColor(getThemeColor(ThemeColorId::HIGHLIGHT)),
    mBorderColor(getThemeColor(ThemeColorId::BORDER)),
    mLabelsColor(getThemeColor(ThemeColorId::LABEL)),
    mLabelsColor2(getThemeColor(ThemeColorId::LABEL_OUTLINE)),
    mSelected(-1),
    mItemPadding(getOption("itemPadding")),
    mBoxSize(getOption("boxSize")),
    mButtonPadding(getOption("buttonPadding", 5)),
    mMinX(180),
    mMinY(345),
    mMaxX(0),
    mMaxY(0),
    mYPadding(0),
    mSelectedTab(0),
    mForing(foring),
    mHaveDefaultPage(false)
{
    const int size = config.getIntValue("fontSize")
        + getOption("tabHeightAdjust", 16);
    mTabs = new TabStrip(this, "equipment", size);
    mTabs->addActionListener(this);
    mTabs->setActionEventId("tab_");

    mYPadding = mTabs->getHeight() + getOption("tabPadding", 2);

    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    if (!mBoxSize)
        mBoxSize = 36;

    // Control that shows the Player
    mPlayerBox->setDimension(Rect(50, 80 + mYPadding, 74, 168));
    mPlayerBox->setPlayer(being);

    if (foring)
        setWindowName("Being equipment");
    else
        setWindowName("Equipment");

    setCloseButton(true);
    setSaveVisible(true);
    setStickyButtonLock(true);

    fillBoxes();
    recalcSize();
    updatePage();
    loadWindowState();
}

void EquipmentWindow::postInit()
{
    Window::postInit();
    const Rect &area = getChildrenArea();
    mUnequip->setPosition(area.width  - mUnequip->getWidth() - mButtonPadding,
        area.height - mUnequip->getHeight() - mButtonPadding);
    mUnequip->setEnabled(false);

    ImageRect rect;
    theme->loadRect(rect, "equipment_background.xml", "", 0, 1);
    mSlotBackground = rect.grid[0];
    mSlotHighlightedBackground = rect.grid[1];
    add(mTabs);
    add(mPlayerBox);
    add(mUnequip);
    enableVisibleSound(true);
    mPlayerBox->setActionEventId("playerbox");
    mPlayerBox->addActionListener(this);
}

EquipmentWindow::~EquipmentWindow()
{
    if (this == beingEquipmentWindow)
    {
        if (mEquipment)
            delete mEquipment->getBackend();
        delete2(mEquipment)
    }
    FOR_EACH (std::vector<EquipmentPage*>::iterator, it, mPages)
    {
        std::vector<EquipmentBox*> &boxes = (*it)->boxes;
        delete_all(boxes);
        boxes.clear();
        delete *it;
    }
    if (mImageSet)
    {
        mImageSet->decRef();
        mImageSet = nullptr;
    }
    if (mSlotBackground)
        mSlotBackground->decRef();
    if (mSlotHighlightedBackground)
        mSlotHighlightedBackground->decRef();
    delete2(mVertexes);
}

void EquipmentWindow::draw(Graphics *graphics)
{
    BLOCK_START("EquipmentWindow::draw")
    // Draw window graphics
    Window::draw(graphics);

    int i = 0;
    Font *const font = getFont();
    const int fontHeight = font->getHeight();
    const std::vector<EquipmentBox*> &boxes = mPages[mSelectedTab]->boxes;

    if (mLastRedraw)
    {
        mVertexes->clear();
        FOR_EACH (std::vector<EquipmentBox*>::const_iterator, it, boxes)
        {
            const EquipmentBox *const box = *it;
            if (!box)
            {
                i ++;
                continue;
            }
            if (i == mSelected)
            {
                graphics->calcTileCollection(mVertexes,
                    mSlotHighlightedBackground,
                    box->x, box->y);
            }
            else
            {
                graphics->calcTileCollection(mVertexes,
                    mSlotBackground,
                    box->x, box->y);
            }
            i ++;
        }
        graphics->finalize(mVertexes);
    }
    graphics->drawTileCollection(mVertexes);

    if (!mEquipment)
    {
        BLOCK_END("EquipmentWindow::draw")
        return;
    }

    i = 0;
    const int projSlot = inventoryHandler->getProjectileSlot();
    for (std::vector<EquipmentBox*>::const_iterator it = boxes.begin(),
         it_end = boxes.end(); it != it_end; ++ it, ++ i)
    {
        const EquipmentBox *const box = *it;
        if (!box)
            continue;
        const Item *const item = mEquipment->getEquipment(i);
        if (item)
        {
            // Draw Item.
            Image *const image = item->getImage();
            if (image)
            {
                image->setAlpha(1.0F);  // Ensure the image is drawn
                                        // with maximum opacity
                graphics->drawImage(image, box->x + mItemPadding,
                    box->y + mItemPadding);
                if (i == projSlot)
                {
                    const std::string str = toString(item->getQuantity());
                    font->drawString(graphics,
                        mLabelsColor,
                        mLabelsColor2,
                        str,
                        box->x + (mBoxSize - font->getWidth(str)) / 2,
                        box->y - fontHeight);
                }
            }
        }
        else if (box->image)
        {
            graphics->drawImage(box->image,
                box->x + mItemPadding,
                box->y + mItemPadding);
        }
    }
    BLOCK_END("EquipmentWindow::draw")
}

void EquipmentWindow::safeDraw(Graphics *graphics)
{
    BLOCK_START("EquipmentWindow::draw")
    // Draw window graphics
    Window::safeDraw(graphics);

    int i = 0;
    Font *const font = getFont();
    const int fontHeight = font->getHeight();
    const std::vector<EquipmentBox*> &boxes = mPages[mSelectedTab]->boxes;

    for (std::vector<EquipmentBox*>::const_iterator it = boxes.begin(),
         it_end = boxes.end(); it != it_end; ++ it, ++ i)
    {
        const EquipmentBox *const box = *it;
        if (!box)
            continue;
        if (i == mSelected)
        {
            graphics->drawImage(mSlotHighlightedBackground,
                box->x, box->y);
        }
        else
        {
            graphics->drawImage(mSlotBackground, box->x, box->y);
        }
    }

    if (!mEquipment)
    {
        BLOCK_END("EquipmentWindow::draw")
        return;
    }

    i = 0;
    const int projSlot = inventoryHandler->getProjectileSlot();
    for (std::vector<EquipmentBox*>::const_iterator it = boxes.begin(),
         it_end = boxes.end(); it != it_end; ++ it, ++ i)
    {
        const EquipmentBox *const box = *it;
        if (!box)
            continue;
        const Item *const item = mEquipment->getEquipment(i);
        if (item)
        {
            // Draw Item.
            Image *const image = item->getImage();
            if (image)
            {
                image->setAlpha(1.0F);  // Ensure the image is drawn
                                        // with maximum opacity
                graphics->drawImage(image, box->x + mItemPadding,
                    box->y + mItemPadding);
                if (i == projSlot)
                {
                    const std::string str = toString(item->getQuantity());
                    font->drawString(graphics,
                        mLabelsColor,
                        mLabelsColor2,
                        str,
                        box->x + (mBoxSize - font->getWidth(str)) / 2,
                        box->y - fontHeight);
                }
            }
        }
        else if (box->image)
        {
            graphics->drawImage(box->image,
                box->x + mItemPadding,
                box->y + mItemPadding);
        }
    }
    BLOCK_END("EquipmentWindow::draw")
}

void EquipmentWindow::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();
    if (eventId == "unequip")
    {
        if (!mEquipment || mSelected == -1)
            return;

        const Item *const item = mEquipment->getEquipment(mSelected);
        PlayerInfo::unequipItem(item, Sfx_true);
        setSelected(-1);
    }
    else if (!eventId.find("tab_"))
    {
        Button *const button = dynamic_cast<Button*>(event.getSource());
        if (!button)
            return;
        mSelectedTab = button->getTag();
        updatePage();
    }
    else if (eventId == "playerbox")
    {
        const DragDropSource src = dragDrop.getSource();
        if (dragDrop.isEmpty() || (src != DragDropSource::Inventory
            && src != DragDropSource::Equipment))
        {
            return;
        }
        Inventory *const inventory = localPlayer
            ? PlayerInfo::getInventory() : nullptr;
        if (!inventory)
            return;
        Item *const item = inventory->findItem(dragDrop.getItem(),
            dragDrop.getItemColor());
        if (!item)
            return;

        if (dragDrop.getSource() == DragDropSource::Inventory)
        {
            if (item->isEquipment() == Equipm_true)
            {
                if (item->isEquipped() == Equipped_false)
                    PlayerInfo::equipItem(item, Sfx_true);
            }
        }
    }
}

void EquipmentWindow::updatePage()
{
    EquipmentPage *const page = mPages[mSelectedTab];
    const Visible visible = fromBool(page->showPlayerBox, Visible);
    mPlayerBox->setVisible(visible);
    if (visible == Visible_true)
    {
        mPlayerBox->setDimension(Rect(page->x, page->y,
            page->width, page->height));
    }
    mRedraw = true;
}

Item *EquipmentWindow::getItem(const int x, const int y) const
{
    if (!mEquipment)
        return nullptr;

    int i = 0;

    std::vector<EquipmentBox*> &boxes = mPages[mSelectedTab]->boxes;
    for (std::vector<EquipmentBox*>::const_iterator it = boxes.begin(),
         it_end = boxes.end(); it != it_end; ++ it, ++ i)
    {
        const EquipmentBox *const box = *it;
        if (!box)
            continue;
        const Rect tRect(box->x, box->y, mBoxSize, mBoxSize);

        if (tRect.isPointInRect(x, y))
            return mEquipment->getEquipment(i);
    }
    return nullptr;
}

void EquipmentWindow::mousePressed(MouseEvent& event)
{
    if (!mEquipment)
    {
        Window::mousePressed(event);
        return;
    }

    const int x = event.getX();
    const int y = event.getY();

    if (event.getButton() == MouseButton::LEFT)
    {
        if (mForing)
        {
            Window::mousePressed(event);
            return;
        }
        // Checks if any of the presses were in the equip boxes.
        int i = 0;

        bool inBox(false);

        std::vector<EquipmentBox*> &boxes = mPages[mSelectedTab]->boxes;
        for (std::vector<EquipmentBox*>::const_iterator it = boxes.begin(),
             it_end = boxes.end(); it != it_end; ++ it, ++ i)
        {
            const EquipmentBox *const box = *it;
            if (!box)
                continue;
            const Item *const item = mEquipment->getEquipment(i);
            const Rect tRect(box->x, box->y, mBoxSize, mBoxSize);

            if (tRect.isPointInRect(x, y))
            {
                inBox = true;
                if (item)
                {
                    event.consume();
                    setSelected(i);
                    dragDrop.dragItem(item, DragDropSource::Equipment);
                    return;
                }
            }
            if (inBox)
                return;
        }
    }
    else if (event.getButton() == MouseButton::RIGHT)
    {
        if (Item *const item = getItem(x, y))
        {
            if (itemPopup)
                itemPopup->setVisible(Visible_false);

            /* Convert relative to the window coordinates to absolute screen
             * coordinates.
             */
            const int mx = x + getX();
            const int my = y + getY();
            if (popupMenu)
            {
                event.consume();
                if (mForing)
                {
                    popupMenu->showUndressPopup(mx, my, mBeing, item);
                }
                else
                {
                    popupMenu->showPopup(this, mx, my, item,
                        InventoryType::INVENTORY);
                }
                return;
            }
        }
    }
    Window::mousePressed(event);
}

void EquipmentWindow::mouseReleased(MouseEvent &event)
{
    Window::mouseReleased(event);
    const DragDropSource src = dragDrop.getSource();
    if (dragDrop.isEmpty() || (src != DragDropSource::Inventory
        && src != DragDropSource::Equipment))
    {
        return;
    }
    Inventory *const inventory = localPlayer
        ? PlayerInfo::getInventory() : nullptr;
    if (!inventory)
        return;

    Item *const item = inventory->findItem(dragDrop.getItem(),
        dragDrop.getItemColor());
    if (!item)
        return;

    if (dragDrop.getSource() == DragDropSource::Inventory)
    {
        if (item->isEquipment() == Equipm_true)
        {
            if (item->isEquipped() == Equipped_false)
                PlayerInfo::equipItem(item, Sfx_true);
        }
    }
    else if (dragDrop.getSource() == DragDropSource::Equipment)
    {
        if (item->isEquipment() == Equipm_true)
        {
            const int x = event.getX();
            const int y = event.getY();
            std::vector<EquipmentBox*> &boxes = mPages[mSelectedTab]->boxes;
            for (std::vector<EquipmentBox*>::const_iterator
                 it = boxes.begin(), it_end = boxes.end();
                 it != it_end; ++ it)
            {
                const EquipmentBox *const box = *it;
                if (!box)
                    continue;
                const Rect tRect(box->x, box->y, mBoxSize, mBoxSize);

                if (tRect.isPointInRect(x, y))
                    return;
            }

            if (item->isEquipped() == Equipped_true)
                PlayerInfo::unequipItem(item, Sfx_true);
        }
    }
    dragDrop.clear();
    dragDrop.deselect();
}

// Show ItemTooltip
void EquipmentWindow::mouseMoved(MouseEvent &event)
{
    Window::mouseMoved(event);

    if (!itemPopup)
        return;

    const int x = event.getX();
    const int y = event.getY();

    const Item *const item = getItem(x, y);

    if (item)
    {
        itemPopup->setItem(item, false);
        itemPopup->position(x + getX(), y + getY());
    }
    else
    {
        itemPopup->setVisible(Visible_false);
    }
}

// Hide ItemTooltip
void EquipmentWindow::mouseExited(MouseEvent &event A_UNUSED)
{
    if (itemPopup)
        itemPopup->setVisible(Visible_false);
}

void EquipmentWindow::setSelected(const int index)
{
    mSelected = index;
    mRedraw = true;
    if (mUnequip)
        mUnequip->setEnabled(mSelected != -1);
    if (itemPopup)
        itemPopup->setVisible(Visible_false);
}

void EquipmentWindow::setBeing(Being *const being)
{
    mPlayerBox->setPlayer(being);
    mBeing = being;
    if (mEquipment)
        delete mEquipment->getBackend();
    delete mEquipment;
    if (!being)
    {
        mEquipment = nullptr;
        return;
    }
    mEquipment = being->getEquipment();
}

void EquipmentWindow::updateBeing(Being *const being)
{
    if (being == mBeing)
        setBeing(being);
}

void EquipmentWindow::resetBeing(const Being *const being)
{
    if (being == mBeing)
        setBeing(nullptr);
}

void EquipmentWindow::fillBoxes()
{
    XML::Document *const doc = new XML::Document(
        paths.getStringValue("equipmentWindowFile"),
        UseResman_true,
        SkipError_false);
    const XmlNodePtr root = doc->rootNode();
    if (!root)
    {
        delete doc;
        fillDefault();
        return;
    }

    if (mImageSet)
        mImageSet->decRef();

    mImageSet = Theme::getImageSetFromTheme(XML::getProperty(
        root, "image", "equipmentbox.png"), 32, 32);

    for_each_xml_child_node(node, root)
    {
        if (xmlNameEqual(node, "page"))
        {
            loadPage(node);
        }
        else if (xmlNameEqual(node, "playerbox"))
        {
            addDefaultPage();
            loadPlayerBox(node, 0);
        }
        else if (xmlNameEqual(node, "slot"))
        {
            addDefaultPage();
            loadSlot(node, mImageSet, 0);
        }
    }
    delete doc;
}

void EquipmentWindow::addDefaultPage()
{
    if (!mHaveDefaultPage)
    {
        mHaveDefaultPage = true;
        addPage(_("default"));
    }
}

void EquipmentWindow::loadPage(const XmlNodePtr node)
{
    if (!node)
        return;
    // TRANSLATORS: unknown equipment page name
    const std::string &name = XML::langProperty(node, "name", _("Unknown"));
    const int page = addPage(name);
    for_each_xml_child_node(childNode, node)
    {
        if (xmlNameEqual(childNode, "playerbox"))
            loadPlayerBox(childNode, page);
        else if (xmlNameEqual(childNode, "slot"))
            loadSlot(childNode, mImageSet, page);
    }
}

void EquipmentWindow::loadPlayerBox(const XmlNodePtr playerBoxNode,
                                    const int page)
{
    EquipmentPage *const data = mPages[page];
    data->x = XML::getProperty(playerBoxNode, "x", 50);
    data->y = XML::getProperty(playerBoxNode, "y", 80) + mYPadding;
    data->width = XML::getProperty(playerBoxNode, "width", 74);
    data->height = XML::getProperty(playerBoxNode, "height", 168);
}

void EquipmentWindow::loadSlot(const XmlNodePtr slotNode,
                               const ImageSet *const imageset,
                               const int page)
{
    if (!imageset)
        return;
    const int slot = parseSlotName(XML::getProperty(slotNode, "name", ""));
    if (slot < 0)
        return;

    const int x = XML::getProperty(slotNode, "x", 0) + getPadding();
    const int y = XML::getProperty(slotNode, "y", 0)
        + getTitleBarHeight() + mYPadding;
    const int imageIndex = XML::getProperty(slotNode, "image", -1);
    Image *image = nullptr;

    if (imageIndex >= 0 && imageIndex < static_cast<signed>(imageset->size()))
        image = imageset->get(imageIndex);

    std::vector<EquipmentBox*> &boxes = mPages[page]->boxes;
    if (boxes[slot])
    {
        EquipmentBox *const box = boxes[slot];
        box->x = x;
        box->y = y;
        box->image = image;
    }
    else
    {
        boxes[slot] = new EquipmentBox(x, y, image);
    }
    if (x < mMinX)
        mMinX = x;
    if (y < mMinY)
        mMinY = y;
    if (x + mBoxSize > mMaxX)
        mMaxX = x + mBoxSize;
    if (y + mBoxSize > mMaxY)
        mMaxY = y + mBoxSize;
}

void EquipmentWindow::prepareSlotNames()
{
    const int sz = sizeof(equipmentSlots) / sizeof(EquipmentSlotMap);
    mSlotNames.clear();
    for (int f = 0; f < sz; f ++)
    {
        const EquipmentSlotMap &slotMap = equipmentSlots[f];
        mSlotNames[slotMap.name] = slotMap.id;
    }
}

int EquipmentWindow::parseSlotName(const std::string &name)
{
    StringIntMapCIter it =  mSlotNames.find(name);
    if (it != mSlotNames.end())
        return (*it).second;
    return -1;
}

void EquipmentWindow::fillDefault()
{
    if (mImageSet)
        mImageSet->decRef();

    addDefaultPage();
    mImageSet = Theme::getImageSetFromTheme(
        "equipmentbox.png", 32, 32);

    addBox(0, 90, 40, 0);     // torso
    addBox(1, 8, 78, 1);      // gloves
    addBox(2, 70, 0, 2);      // hat
    addBox(3, 50, 253, 3);    // pants
    addBox(4, 90, 253, 4);    // boots
    addBox(5, 8, 213, 5);     // FREE
    addBox(6, 129, 213, 6);   // wings
    addBox(7, 50, 40, 5);     // scarf
    addBox(8, 8, 168, 7);     // weapon
    addBox(9, 129, 168, 8);   // shield
    addBox(10, 129, 78, 9);   // ammo
    addBox(11, 8, 123, 5);    // amulet
    addBox(12, 129, 123, 5);  // ring
}

void EquipmentWindow::addBox(const int idx, int x, int y, const int imageIndex)
{
    Image *image = nullptr;

    if (mImageSet && imageIndex >= 0 && imageIndex
        < static_cast<signed>(mImageSet->size()))
    {
        image = mImageSet->get(imageIndex);
    }

    x += getPadding();
    y += getTitleBarHeight() + mYPadding;
    std::vector<EquipmentBox*> &boxes = mPages[0]->boxes;
    boxes[idx] = new EquipmentBox(x, y, image);

    if (x < mMinX)
        mMinX = x;
    if (y < mMinY)
        mMinY = y;
    if (x + mBoxSize > mMaxX)
        mMaxX = x + mBoxSize;
    if (y + mBoxSize > mMaxY)
        mMaxY = y + mBoxSize;
}

void EquipmentWindow::recalcSize()
{
    mMaxX += mMinX;
    mMaxY += mMinY;
    mTabs->setWidth(mMaxX);
    mMaxY += mUnequip->getHeight() - mYPadding;
    setDefaultSize(mMaxX, mMaxY, ImageRect::CENTER);
}

int EquipmentWindow::addPage(const std::string &name)
{
    EquipmentPage *const page = new EquipmentPage;
    mPages.push_back(page);
    std::vector<EquipmentBox*> &boxes = page->boxes;

    boxes.reserve(BOX_COUNT);
    for (int f = 0; f < BOX_COUNT; f ++)
        boxes.push_back(nullptr);

    mTabs->addButton(name, name, false);
    return static_cast<int>(mPages.size()) - 1;
}
