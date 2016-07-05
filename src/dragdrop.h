/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2016  The ManaPlus Developers
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

#ifndef DRAGDROP_H
#define DRAGDROP_H

#include "resources/item/item.h"

#include "itemsoundmanager.h"
#include "textcommand.h"

#include "enums/dragdropsource.h"

#include "gui/widgets/skilldata.h"
#include "gui/widgets/skillinfo.h"

#include "resources/image/image.h"

#include "localconsts.h"

class DragDrop final
{
    public:
        DragDrop(Item *const item,
                 const DragDropSourceT source) :
            mItemImage(item ? item->getImage() : nullptr),
            mText(),
            mSource(source),
            mItem(item ? item->getId() : 0),
            mSelItem(0),
            mTag(-1),
            mItemColor(item ? item->getColor() : ItemColor_one),
            mSelItemColor(ItemColor_one)
        {
            if (mItemImage)
                mItemImage->incRef();
        }

        A_DELETE_COPY(DragDrop)

        ~DragDrop()
        {
            if (mItemImage)
                mItemImage->decRef();
        }

        int getItem() const
        { return mItem; }

        ItemColor getItemColor() const
        { return mItemColor; }

        void setItemColor(const ItemColor color)
        { mItemColor = color; }

        Image *getItemImage()
        { return mItemImage; }

        DragDropSourceT getSource() const
        { return mSource; }

        void dragItem(const Item *const item,
                      const DragDropSourceT source,
                      const int tag = 0)
        {
            if (mItemImage)
                mItemImage->decRef();

            mText.clear();
            if (item)
            {
                mItem = item->getId();
                mItemColor = item->getColor();
                mItemImage = item->getImage();
                if (mItemImage)
                    mItemImage->incRef();
                mSource = source;
                mTag = tag;
                ItemSoundManager::playSfx(item, ItemSoundEvent::TAKE);
            }
            else
            {
                mItem = 0;
                mItemColor = ItemColor_one;
                mItemImage = nullptr;
                mSource = DragDropSource::Empty;
                mTag = -1;
            }
        }

        void dragCommand(const TextCommand *const command,
                         const DragDropSourceT source,
                         const int tag = 0)
        {
            if (mItemImage)
                mItemImage->decRef();
            mItem = 0;
            mItemColor = ItemColor_one;

            if (command)
            {
                mText = command->getSymbol();
                mItemImage = command->getImage();
                if (mItemImage)
                {
                    mItemImage->incRef();
                }
                else if (mText.empty())
                {
                    mSource = source;
                    mTag = -1;
                    return;
                }
                mItem = command->getId();
            }
            else
            {
                mText.clear();
                mItemImage = nullptr;
            }
            mSource = source;
            mTag = tag;
        }

        void dragSkill(const SkillInfo *const info,
                       const DragDropSourceT source,
                       const int tag = 0)
        {
            if (mItemImage)
                mItemImage->decRef();
            mItem = 0;
            mItemColor = ItemColor_zero;
            mText.clear();
            mItemImage = nullptr;
            mSource = DragDropSource::Empty;
            mTag = -1;
            if (info)
            {
                const SkillData *const data = info->data;
                if (data)
                {
                    mText = data->name;
                    mItemImage = data->icon;
                    if (mItemImage)
                        mItemImage->incRef();
                    mSource = source;
                    mTag = tag;
                }
                mItem = info->id;
                mItemColor = fromInt(info->selectedLevel,
                    ItemColor);
            }
        }

        void clear()
        {
            if (mItemImage)
                mItemImage->decRef();
            mItem = 0;
            mItemColor = ItemColor_one;
            mItemImage = nullptr;
            mSource = DragDropSource::Empty;
            mText.clear();
            mTag = -1;
        }

        bool isEmpty() const
        { return mSource == DragDropSource::Empty; }

        void select(const Item *const item)
        {
            if (item)
            {
                mSelItem = item->getId();
                mSelItemColor = item->getColor();
            }
            else
            {
                mSelItem = 0;
                mSelItemColor = ItemColor_one;
            }
        }

        void deselect()
        {
            mSelItem = 0;
            mSelItemColor = ItemColor_one;
        }

        int getSelected() const
        { return mSelItem; }

        ItemColor getSelectedColor() const
        { return mSelItemColor; }

        bool isSelected() const
        { return mSelItem > 0; }

        void clearItem(const Item *const item A_UNUSED)
        {
        }

        const std::string &getText() const
        { return mText; }

        int getTag() const
        { return mTag; }

        void setItem(const int item)
        { mItem = item; }

        bool isSourceItemContainer() const
        {
            return mSource == DragDropSource::Inventory
                || mSource == DragDropSource::Storage
#ifdef EATHENA_SUPPORT
                || mSource == DragDropSource::Cart
                || mSource == DragDropSource::Craft
#endif
                || mSource == DragDropSource::Trade
                || mSource == DragDropSource::Outfit
                || mSource == DragDropSource::Ground
                || mSource == DragDropSource::Drop;
        }

    private:
        Image *mItemImage;
        std::string mText;
        DragDropSourceT mSource;
        int mItem;
        int mSelItem;
        int mTag;
        ItemColor mItemColor;
        ItemColor mSelItemColor;
};

extern DragDrop dragDrop;

#endif  // DRAGDROP_H
