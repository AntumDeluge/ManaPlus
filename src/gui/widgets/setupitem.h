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

#ifndef GUI_WIDGETS_SETUPITEM_H
#define GUI_WIDGETS_SETUPITEM_H

#include "enums/simpletypes/mainconfig.h"

#include "gui/widgets/widget2.h"

#include "listeners/actionlistener.h"

#include <list>

class Button;
class CheckBox;
class Configuration;
class DropDown;
class EditDialog;
class HorizontContainer;
class IntTextField;
class Label;
class ListModel;
class SetupTabScroll;
class Slider;
class SliderList;
class TextField;

class SetupItem notfinal : public ActionListener,
                           public Widget2
{
    public:
        enum
        {
            VBOOL = 0,
            VSTR,
            VINT,
            VNONE
        };

        A_DELETE_COPY(SetupItem)

        virtual ~SetupItem();

        void load();

        virtual void save();

        virtual void fromWidget() = 0;

        virtual void toWidget() = 0;

        void setWidget(Widget *widget)
        { mWidget = widget; }

        Widget *getWidget() const A_WARN_UNUSED
        { return mWidget; }

        Configuration *getConfig() const RETURNS_NONNULL A_WARN_UNUSED;

        virtual std::string getActionEventId() const A_WARN_UNUSED;

        void action(const ActionEvent &event) override;

        virtual void doAction();

        virtual void apply(const std::string &eventName);

        virtual void cancel(const std::string &eventName);

        virtual void externalUpdated(const std::string &eventName);

        virtual void externalUnloaded(const std::string &eventName);

        MainConfig isMainConfig() const A_WARN_UNUSED
        { return mMainConfig; }

        void fixFirstItemSize(Widget *const widget);

        virtual void rereadValue();

        void setValue(const std::string &str)
        { mValue = str; }

        std::string getValue() const
        { return mValue; }

        std::string getEventName() const
        { return mEventName; }

    protected:
        SetupItem(const std::string &restrict text,
                  const std::string &restrict description,
                  const std::string &restrict keyName,
                  SetupTabScroll *restrict const parent,
                  const std::string &restrict eventName,
                  const MainConfig mainConfig);

        SetupItem(const std::string &restrict text,
                  const std::string &restrict description,
                  const std::string &restrict keyName,
                  SetupTabScroll *restrict const parent,
                  const std::string &restrict eventName,
                  const std::string &def,
                  const MainConfig mainConfig);

        std::string mText;

        std::string mDescription;

        std::string mKeyName;

        SetupTabScroll *mParent;

        std::string mEventName;

        std::string mValue;

        std::string mDefault;

        Widget *mWidget;

        std::list<Widget*> mTempWidgets;

        int mValueType;

        MainConfig mMainConfig;

        bool mUseDefault;
};

class SetupItemCheckBox final : public SetupItem
{
    public:
        SetupItemCheckBox(const std::string &restrict text,
                          const std::string &restrict description,
                          const std::string &restrict keyName,
                          SetupTabScroll *restrict const parent,
                          const std::string &restrict eventName,
                          const MainConfig mainConfig = MainConfig_true);

        SetupItemCheckBox(const std::string &restrict text,
                          const std::string &restrict description,
                          const std::string &restrict keyName,
                          SetupTabScroll *restrict const parent,
                          const std::string &restrict eventName,
                          const std::string &restrict def,
                          const MainConfig mainConfig = MainConfig_true);

        A_DELETE_COPY(SetupItemCheckBox)

        ~SetupItemCheckBox();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

    protected:
        CheckBox *mCheckBox;
};

class SetupItemTextField final : public SetupItem
{
    public:
        SetupItemTextField(const std::string &restrict text,
                           const std::string &restrict description,
                           const std::string &restrict keyName,
                           SetupTabScroll *restrict const parent,
                           const std::string &restrict eventName,
                           const MainConfig mainConfig = MainConfig_true,
                           const bool useBase64 = false);

        SetupItemTextField(const std::string &restrict text,
                           const std::string &restrict description,
                           const std::string &restrict keyName,
                           SetupTabScroll *restrict const parent,
                           const std::string &restrict eventName,
                           const std::string &restrict def,
                           const MainConfig mainConfig = MainConfig_true,
                           const bool useBase64 = false);

        A_DELETE_COPY(SetupItemTextField)

        ~SetupItemTextField();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

        void action(const ActionEvent &event) override final;

        void apply(const std::string &eventName) override final;

        void cancel(const std::string &eventName A_UNUSED) override final;

        void externalUpdated(const std::string &eventName A_UNUSED)
                             override final;

        void rereadValue() override final;

        void save() override final;

        void setUseBase64(const bool b)
        { mUseBase64 = b; }

    protected:
        HorizontContainer *mHorizont;
        Label *mLabel;
        TextField *mTextField;
        Button *mButton;
        EditDialog *mEditDialog;
        bool mUseBase64;
};

class SetupItemIntTextField final : public SetupItem
{
    public:
        SetupItemIntTextField(const std::string &restrict text,
                              const std::string &restrict description,
                              const std::string &restrict keyName,
                              SetupTabScroll *restrict const parent,
                              const std::string &restrict eventName,
                              const int min, const int max,
                              const MainConfig mainConfig = MainConfig_true);

        SetupItemIntTextField(const std::string &restrict text,
                              const std::string &restrict description,
                              const std::string &restrict keyName,
                              SetupTabScroll *restrict const parent,
                              const std::string &restrict eventName,
                              const int min, const int max,
                              const std::string &restrict def,
                              const MainConfig mainConfig = MainConfig_true);

        A_DELETE_COPY(SetupItemIntTextField)

        ~SetupItemIntTextField();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

        void action(const ActionEvent &event) override final;

        void apply(const std::string &eventName) override final;

    protected:
        HorizontContainer *mHorizont;
        Label *mLabel;
        IntTextField *mTextField;
        Button *mButton;
        EditDialog *mEditDialog;
        int mMin;
        int mMax;
};

class SetupItemLabel final : public SetupItem
{
    public:
        SetupItemLabel(const std::string &restrict text,
                       const std::string &restrict description,
                       SetupTabScroll *restrict const parent,
                       const bool separator = true);

        A_DELETE_COPY(SetupItemLabel)

        ~SetupItemLabel();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

        void action(const ActionEvent &event) override final;

        void apply(const std::string &eventName) override final;

    protected:
        Label *mLabel;
        bool mIsSeparator;
};

class SetupItemDropDown final : public SetupItem
{
    public:
        SetupItemDropDown(const std::string &restrict text,
                          const std::string &restrict description,
                          const std::string &restrict keyName,
                          SetupTabScroll *restrict const parent,
                          const std::string &restrict eventName,
                          ListModel *restrict const model,
                          const int width,
                          const MainConfig mainConfig = MainConfig_true);

        SetupItemDropDown(const std::string &restrict text,
                          const std::string &restrict description,
                          const std::string &restrict keyName,
                          SetupTabScroll *restrict const parent,
                          const std::string &restrict eventName,
                          ListModel *restrict const model,
                          const int width,
                          const std::string &restrict def,
                          const MainConfig mainConfig = MainConfig_true);

        A_DELETE_COPY(SetupItemDropDown)

        ~SetupItemDropDown();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

    protected:
        HorizontContainer *mHorizont;
        Label *mLabel;
        ListModel *mModel;
        DropDown *mDropDown;
        int mWidth;
};

class SetupItemSlider final : public SetupItem
{
    public:
        SetupItemSlider(const std::string &restrict text,
                        const std::string &restrict description,
                        const std::string &restrict keyName,
                        SetupTabScroll *restrict const parent,
                        const std::string &restrict eventName,
                        const double min,
                        const double max,
                        const double step,
                        const int width,
                        const bool onTheFly,
                        const MainConfig mainConfig);

        SetupItemSlider(const std::string &restrict text,
                        const std::string &restrict description,
                        const std::string &restrict keyName,
                        SetupTabScroll *restrict const parent,
                        const std::string &restrict eventName,
                        const double min,
                        const double max,
                        const double step,
                        const std::string &restrict def,
                        const int width,
                        const bool onTheFly,
                        const MainConfig mainConfig);

        A_DELETE_COPY(SetupItemSlider)

        ~SetupItemSlider();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

        void action(const ActionEvent &event) override final;

        void apply(const std::string &eventName) override final;

        void updateLabel();

    protected:
        HorizontContainer *mHorizont;
        Label *mLabel;
        Slider *mSlider;
        double mMin;
        double mMax;
        double mStep;
        int mWidth;
        bool mOnTheFly;
};

typedef std::vector<std::string> SetupItemNames;
typedef SetupItemNames::iterator SetupItemNamesIter;
typedef SetupItemNames::const_iterator SetupItemNamesConstIter;

class SetupItemSlider2 final : public SetupItem
{
    public:
        SetupItemSlider2(const std::string &restrict text,
                         const std::string &restrict description,
                         const std::string &restrict keyName,
                         SetupTabScroll *restrict const parent,
                         const std::string &restrict eventName,
                         const int min, const int max, const int step,
                         SetupItemNames *restrict const values,
                         const bool onTheFly,
                         const MainConfig mainConfig,
                         const bool doNotAlign);

        SetupItemSlider2(const std::string &restrict text,
                         const std::string &restrict description,
                         const std::string &restrict keyName,
                         SetupTabScroll *restrict const parent,
                         const std::string &restrict eventName,
                         const int min, const int max, const int step,
                         SetupItemNames *restrict const values,
                         const std::string &restrict def,
                         const bool onTheFly,
                         const MainConfig mainConfig,
                         const bool doNotAlign);

        A_DELETE_COPY(SetupItemSlider2)

        ~SetupItemSlider2();

        void createControls();

        void fromWidget() override final;

        void toWidget() override final;

        void action(const ActionEvent &event) override final;

        void apply(const std::string &eventName) override final;

        void setInvertValue(const int v);

    protected:
        void updateLabel();

        int getMaxWidth();

        HorizontContainer *mHorizont;
        Label *mLabel;
        Label *mLabel2;
        Slider *mSlider;
        SetupItemNames *mValues;
        int mMin;
        int mMax;
        int mStep;
        int mInvertValue;
        bool mInvert;
        bool mOnTheFly;
        bool mDoNotAlign;
};

class SetupItemSliderList notfinal : public SetupItem
{
    public:
        A_DELETE_COPY(SetupItemSliderList)

        virtual ~SetupItemSliderList();

        void createControls();

        void fromWidget() override;

        void toWidget() override;

        void action(const ActionEvent &event) override;

        void apply(const std::string &eventName) override final;

        virtual void addMoreControls() = 0;

    protected:
        SetupItemSliderList(const std::string &restrict text,
                            const std::string &restrict description,
                            const std::string &restrict keyName,
                            SetupTabScroll *restrict const parent,
                            const std::string &restrict eventName,
                            ListModel *restrict const model,
                            const int width = 150,
                            const bool onTheFly = false,
                            const MainConfig mainConfig = MainConfig_true);

        SetupItemSliderList(const std::string &restrict text,
                            const std::string &restrict description,
                            const std::string &restrict keyName,
                            SetupTabScroll *restrict const parent,
                            const std::string &restrict eventName,
                            ListModel *restrict const model,
                            const std::string &restrict def,
                            const int width = 150,
                            const bool onTheFly = false,
                            const MainConfig mainConfig = MainConfig_true);

        HorizontContainer *mHorizont;
        Label *mLabel;
        SliderList *mSlider;
        ListModel *mModel;
        int mWidth;
        bool mOnTheFly;
};

class SetupItemSound final : public SetupItemSliderList
{
    public:
        SetupItemSound(const std::string &restrict text,
                       const std::string &restrict description,
                       const std::string &restrict keyName,
                       SetupTabScroll *restrict const parent,
                       const std::string &restrict eventName,
                       ListModel *restrict const model,
                       const int width = 150,
                       const bool onTheFly = false,
                       const MainConfig mainConfig = MainConfig_true);

        A_DELETE_COPY(SetupItemSound)

        void action(const ActionEvent &event) override final;

        void addMoreControls() override final;

    protected:
        Button *mButton;
};

class SetupItemSliderInt final : public SetupItemSliderList
{
    public:
        SetupItemSliderInt(const std::string &restrict text,
                           const std::string &restrict description,
                           const std::string &restrict keyName,
                           SetupTabScroll *restrict const parent,
                           const std::string &restrict eventName,
                           ListModel *restrict const model,
                           const int min,
                           const int width = 150,
                           const bool onTheFly = false,
                           const MainConfig mainConfig = MainConfig_true);

        A_DELETE_COPY(SetupItemSliderInt)

        void addMoreControls() override final;

        void fromWidget() override final;

        void toWidget() override final;

    protected:
        int mMin;
};

#endif  // GUI_WIDGETS_SETUPITEM_H
