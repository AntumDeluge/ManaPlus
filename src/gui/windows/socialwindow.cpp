/*
 *  The ManaPlus Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "gui/windows/socialwindow.h"

#include "gui/windows/okdialog.h"
#include "gui/windows/setupwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/tabbedarea.h"

#include "gui/widgets/tabs/socialattacktab.h"
#include "gui/widgets/tabs/socialfriendstab.h"
#include "gui/widgets/tabs/socialguildtab.h"
#include "gui/widgets/tabs/socialguildtab2.h"
#include "gui/widgets/tabs/socialnavigationtab.h"
#include "gui/widgets/tabs/socialpartytab.h"
#include "gui/widgets/tabs/socialpickuptab.h"
#include "gui/widgets/tabs/socialplayerstab.h"

#include "net/serverfeatures.h"

#ifdef TMWA_SUPPORT
#include "net/tmwa/guildmanager.h"
#endif

#include "debug.h"

SocialWindow *socialWindow = nullptr;

SocialWindow::SocialWindow() :
    // TRANSLATORS: social window name
    Window(_("Social"), Modal_false, nullptr, "social.xml"),
    ActionListener(),
    PlayerRelationsListener(),
    mGuildInvited(0),
    mGuildAcceptDialog(nullptr),
    mGuildCreateDialog(nullptr),
    mPartyInviter(),
    mGuilds(),
    mParties(),
    mPartyAcceptDialog(nullptr),
    mAttackFilter(nullptr),
    mPickupFilter(nullptr),
    // TRANSLATORS: here P is title for visible players tab in social window
    mPlayers(new SocialPlayersTab(this, _("P"),
        getOptionBool("showtabbackground"))),
    mNavigation(new SocialNavigationTab(this,
        getOptionBool("showtabbackground"))),
    // TRANSLATORS: here F is title for friends tab in social window
    mFriends(new SocialFriendsTab(this, _("F"),
        getOptionBool("showtabbackground"))),
    // TRANSLATORS: social window button
    mCreateButton(new Button(this, _("Create"), "create", this)),
    mCountLabel(new Label(this, "1000 / 1000")),
    mTabs(CREATEWIDGETR(TabbedArea, this)),
    mMap(nullptr),
    mLastUpdateTime(0),
    mPartyId(0),
    mNeedUpdate(false),
    mProcessedPortals(false)
{
}

void SocialWindow::postInit()
{
    Window::postInit();
    setWindowName("Social");
    setVisible(Visible_false);
    setSaveVisible(true);
    setResizable(true);
    setSaveVisible(true);
    setCloseButton(true);
    setStickyButtonLock(true);

    setMinWidth(120);
    setMinHeight(55);
    setDefaultSize(590, 200, 180, 300);
    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    place(0, 0, mCreateButton);
    place(0, 1, mCountLabel);
    place(0, 2, mTabs, 2, 4);

    widgetResized(Event(nullptr));

    loadWindowState();

    mTabs->addTab(mPlayers, mPlayers->mScroll);
    mTabs->addTab(mFriends, mFriends->mScroll);
    mTabs->addTab(mNavigation, mNavigation->mScroll);

    if (config.getBoolValue("enableAttackFilter"))
    {
        mAttackFilter = new SocialAttackTab(this,
            getOptionBool("showtabbackground"));
        mTabs->addTab(mAttackFilter, mAttackFilter->mScroll);
    }
    else
    {
        mAttackFilter = nullptr;
    }

    if (config.getBoolValue("enablePickupFilter"))
    {
        mPickupFilter = new SocialPickupTab(this,
            getOptionBool("showtabbackground"));
        mTabs->addTab(mPickupFilter, mPickupFilter->mScroll);
    }
    else
    {
        mPickupFilter = nullptr;
    }

    if (localPlayer && localPlayer->getParty())
        addTab(localPlayer->getParty());

    if (localPlayer && localPlayer->getGuild())
        addTab(localPlayer->getGuild());

    enableVisibleSound(true);
    updateButtons();
    player_relations.addListener(this);
}

SocialWindow::~SocialWindow()
{
    player_relations.removeListener(this);
    if (mGuildAcceptDialog)
    {
        mGuildAcceptDialog->close();
        mGuildAcceptDialog->scheduleDelete();
        mGuildAcceptDialog = nullptr;

        mGuildInvited = 0;
    }

    if (mPartyAcceptDialog)
    {
        mPartyAcceptDialog->close();
        mPartyAcceptDialog->scheduleDelete();
        mPartyAcceptDialog = nullptr;

        mPartyInviter.clear();
    }
    delete2(mPlayers);
    delete2(mNavigation);
    delete2(mAttackFilter);
    delete2(mPickupFilter);
    delete2(mFriends);
}

bool SocialWindow::addTab(Guild *const guild)
{
    if (!guild)
        return false;

    if (mGuilds.find(guild) != mGuilds.end())
        return false;

    SocialTab *tab = nullptr;
    if (guild->getServerGuild())
    {
        tab = new SocialGuildTab(this, guild,
            getOptionBool("showtabbackground"));
    }
    else
    {
        tab = new SocialGuildTab2(this, guild,
            getOptionBool("showtabbackground"));
    }

    mGuilds[guild] = tab;
    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Guild *const guild)
{
    const GuildMap::iterator it = mGuilds.find(guild);
    if (it == mGuilds.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mGuilds.erase(it);

    updateButtons();

    return true;
}

bool SocialWindow::addTab(Party *const party)
{
    if (mParties.find(party) != mParties.end())
        return false;

    SocialPartyTab *const tab = new SocialPartyTab(this, party,
        getOptionBool("showtabbackground"));
    mParties[party] = tab;

    mTabs->addTab(tab, tab->mScroll);

    updateButtons();

    return true;
}

bool SocialWindow::removeTab(Party *const party)
{
    const PartyMap::iterator it = mParties.find(party);
    if (it == mParties.end())
        return false;

    mTabs->removeTab(it->second);
    delete it->second;
    mParties.erase(it);

    updateButtons();

    return true;
}

void SocialWindow::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();

    if (event.getSource() == mPartyAcceptDialog)
    {
        if (eventId == "yes")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Accepted party invite from %s."),
                    mPartyInviter.c_str()),
                    ChatMsgType::BY_SERVER);
            }
            partyHandler->inviteResponse(mPartyId, true);
        }
        else if (eventId == "no")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Rejected party invite from %s."),
                    mPartyInviter.c_str()),
                    ChatMsgType::BY_SERVER);
            }
            partyHandler->inviteResponse(mPartyId, false);
        }

        mPartyInviter.clear();
        mPartyAcceptDialog = nullptr;
    }
    else if (event.getSource() == mGuildAcceptDialog)
    {
        if (eventId == "yes")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Accepted guild invite from %s."),
                    mPartyInviter.c_str()),
                    ChatMsgType::BY_SERVER);
            }
#ifdef TMWA_SUPPORT
            if (!guildManager || !GuildManager::getEnableGuildBot())
                guildHandler->inviteResponse(mGuildInvited, true);
            else
                guildManager->inviteResponse(true);
#else
            guildHandler->inviteResponse(mGuildInvited, true);
#endif
        }
        else if (eventId == "no")
        {
            if (localChatTab)
            {
                localChatTab->chatLog(
                    // TRANSLATORS: chat message
                    strprintf(_("Rejected guild invite from %s."),
                    mPartyInviter.c_str()),
                    ChatMsgType::BY_SERVER);
            }
#ifdef TMWA_SUPPORT
            if (!guildManager || !GuildManager::getEnableGuildBot())
                guildHandler->inviteResponse(mGuildInvited, false);
            else
                guildManager->inviteResponse(false);
#else
            guildHandler->inviteResponse(mGuildInvited, false);
#endif
        }

        mGuildInvited = 0;
        mGuildAcceptDialog = nullptr;
    }
}

void SocialWindow::showGuildInvite(const std::string &restrict guildName,
                                   const int guildId,
                                   const std::string &restrict inviterName)
{
    // check there isnt already an invite showing
    if (mGuildInvited != 0)
    {
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(_("Received guild request, but one already "
                "exists."),
                ChatMsgType::BY_SERVER);
        }
        return;
    }

    const std::string msg = strprintf(
        // TRANSLATORS: chat message
        _("%s has invited you to join the guild %s."),
        inviterName.c_str(), guildName.c_str());

    if (localChatTab)
        localChatTab->chatLog(msg, ChatMsgType::BY_SERVER);

    CREATEWIDGETV(mGuildAcceptDialog, ConfirmDialog,
        // TRANSLATORS: guild invite message
        _("Accept Guild Invite"),
        msg,
        SOUND_REQUEST,
        false,
        Modal_false,
        this);
    mGuildAcceptDialog->addActionListener(this);
    mGuildInvited = guildId;
}

void SocialWindow::showPartyInvite(const std::string &restrict partyName,
                                   const std::string &restrict inviter,
                                   const int partyId)
{
    // check there isnt already an invite showing
    if (!mPartyInviter.empty())
    {
        if (localChatTab)
        {
            // TRANSLATORS: chat message
            localChatTab->chatLog(_("Received party request, but one already "
                "exists."),
                ChatMsgType::BY_SERVER);
        }
        return;
    }

    std::string msg;
    if (inviter.empty())
    {
        if (partyName.empty())
        {
            // TRANSLATORS: party invite message
            msg = _("You have been invited you to join a party.");
        }
        else
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("You have been invited to join the %s party."),
                partyName.c_str());
        }
    }
    else
    {
        if (partyName.empty())
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("%s has invited you to join their party."),
                inviter.c_str());
        }
        else
        {
            // TRANSLATORS: party invite message
            msg = strprintf(_("%s has invited you to join the %s party."),
                inviter.c_str(), partyName.c_str());
        }
    }

    if (localChatTab)
        localChatTab->chatLog(msg, ChatMsgType::BY_SERVER);

    // show invite
    CREATEWIDGETV(mPartyAcceptDialog, ConfirmDialog,
        // TRANSLATORS: party invite message
        _("Accept Party Invite"),
        msg,
        SOUND_REQUEST,
        false,
        Modal_false,
        this);
    mPartyAcceptDialog->addActionListener(this);
    mPartyInviter = inviter;
    mPartyId = partyId;
}

void SocialWindow::updateActiveList()
{
    mNeedUpdate = true;
}

void SocialWindow::slowLogic()
{
    BLOCK_START("SocialWindow::slowLogic")
    const unsigned int nowTime = cur_time;
    if (mNeedUpdate && nowTime - mLastUpdateTime > 1)
    {
        mPlayers->updateList();
        mFriends->updateList();
        mNeedUpdate = false;
        mLastUpdateTime = nowTime;
    }
    else if (nowTime - mLastUpdateTime > 5)
    {
        mPlayers->updateList();
        mNeedUpdate = false;
        mLastUpdateTime = nowTime;
    }
    BLOCK_END("SocialWindow::slowLogic")
}

void SocialWindow::updateAvatar(const std::string &name)
{
    mPlayers->updateAvatar(name);
}

void SocialWindow::resetDamage(const std::string &name)
{
    mPlayers->resetDamage(name);
}

void SocialWindow::updateButtons()
{
//    if (!mTabs)
//        return;

//    const bool hasTabs = mTabs->getNumberOfTabs() > 0;
}

void SocialWindow::updatePortals()
{
    if (mNavigation)
        mNavigation->updateList();
}

void SocialWindow::updatePortalNames()
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->updateNames();
}

void SocialWindow::selectPortal(const unsigned num)
{
    if (mNavigation)
        mNavigation->selectIndex(num);
}

int SocialWindow::getPortalIndex(const int x, const int y)
{
    if (mNavigation)
    {
        return static_cast<SocialNavigationTab*>(
            mNavigation)->getPortalIndex(x, y);
    }
    else
    {
        return -1;
    }
}

void SocialWindow::addPortal(const int x, const int y)
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->addPortal(x, y);
}

void SocialWindow::removePortal(const int x, const int y)
{
    if (mNavigation)
        static_cast<SocialNavigationTab*>(mNavigation)->removePortal(x, y);
}

void SocialWindow::nextTab()
{
    if (mTabs)
        mTabs->selectNextTab();
}

void SocialWindow::prevTab()
{
    if (mTabs)
        mTabs->selectPrevTab();
}

void SocialWindow::updateAttackFilter()
{
    if (mAttackFilter)
        mAttackFilter->updateList();
}

void SocialWindow::updatePickupFilter()
{
    if (mPickupFilter)
        mPickupFilter->updateList();
}

void SocialWindow::updateParty()
{
    if (!localPlayer)
        return;

    Party *const party = localPlayer->getParty();
    if (party)
    {
        const PartyMap::iterator it = mParties.find(party);
        if (it != mParties.end())
        {
            SocialTab *const tab = (*it).second;
            tab->buildCounter();
        }
    }
}

void SocialWindow::widgetResized(const Event &event)
{
    Window::widgetResized(event);
    if (mTabs)
        mTabs->adjustSize();
}

void SocialWindow::setCounter(const SocialTab *const tab,
                              const std::string &str)
{
    if (mTabs->getSelectedTab() == tab)
    {
        mCountLabel->setCaption(str);
        mCountLabel->adjustSize();
    }
}

void SocialWindow::updateGuildCounter(const int online, const int total)
{
    if (!localPlayer)
        return;

    Guild *const guild = localPlayer->getGuild();
    if (guild)
    {
        const GuildMap::iterator it = mGuilds.find(guild);
        if (it != mGuilds.end())
        {
            SocialTab *const tab = (*it).second;
            tab->buildCounter(online, total);
        }
    }
}

void SocialWindow::updatedPlayer(const std::string &name A_UNUSED)
{
    mNeedUpdate = true;
}

void SocialWindow::updateAll()
{
    mNeedUpdate = true;
}

#ifdef USE_PROFILER
void SocialWindow::logicChildren()
{
    BLOCK_START("SocialWindow::logicChildren")
    BasicContainer::logicChildren();
    BLOCK_END("SocialWindow::logicChildren")
}
#endif
