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

#ifndef GUI_SETUP_AUDIO_H
#define GUI_SETUP_AUDIO_H

#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"

#include <guichan/actionlistener.hpp>

class Setup_Audio : public SetupTab, public gcn::ActionListener
{
    public:
        Setup_Audio();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

    private:
        int mMusicVolume, mSfxVolume;
        bool mAudioEnabled, mGameSoundEnabled, mGuiSoundEnabled;
        bool mMusicEnabled, mMumbleEnabled;
        bool mDownloadEnabled;

        gcn::CheckBox *mAudioCheckBox, *mGameSoundCheckBox, *mGuiSoundCheckBox;
        gcn::CheckBox *mMusicCheckBox, *mMumbleCheckBox;
        gcn::CheckBox *mDownloadMusicCheckBox;
        gcn::Slider *mSfxSlider, *mMusicSlider;
};

#endif
