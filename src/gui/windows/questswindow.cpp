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

#include "gui/windows/questswindow.h"

#include "actormanager.h"
#include "configuration.h"
#include "effectmanager.h"

#include "being/localplayer.h"

#include "gui/gui.h"

#include "gui/fonts/font.h"

#include "gui/models/questsmodel.h"

#include "gui/windows/setupwindow.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/containerplacer.h"
#include "gui/widgets/createwidget.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/layouttype.h"
#include "gui/widgets/extendedlistbox.h"
#include "gui/widgets/itemlinkhandler.h"
#include "gui/widgets/scrollarea.h"

#include "utils/delete2.h"
#include "utils/dtor.h"
#include "utils/files.h"
#include "utils/gettext.h"

#include "utils/translation/podict.h"

#include "resources/beingcommon.h"
#include "resources/questeffect.h"
#include "resources/questitem.h"
#include "resources/questtype.h"

#include "resources/map/map.h"

#include "debug.h"

QuestsWindow *questsWindow = nullptr;

QuestsWindow::QuestsWindow() :
    // TRANSLATORS: quests window name
    Window(_("Quests"), Modal_false, nullptr, "quests.xml"),
    ActionListener(),
    mQuestsModel(new QuestsModel),
    mQuestsListBox(CREATEWIDGETR(ExtendedListBox,
        this, mQuestsModel, "extendedlistbox.xml")),
    mQuestScrollArea(new ScrollArea(this, mQuestsListBox,
        getOptionBool("showlistbackground"), "quests_list_background.xml")),
    mItemLinkHandler(new ItemLinkHandler),
    mText(new BrowserBox(this, BrowserBox::AUTO_WRAP, true, "browserbox.xml")),
    mTextScrollArea(new ScrollArea(this, mText,
        getOptionBool("showtextbackground"), "quests_text_background.xml")),
    // TRANSLATORS: quests window button
    mCloseButton(new Button(this, _("Close"), "close", this)),
    mCompleteIcon(Theme::getImageFromThemeXml("complete_icon.xml", "")),
    mIncompleteIcon(Theme::getImageFromThemeXml("incomplete_icon.xml", "")),
    mVars(),
    mQuests(),
    mAllEffects(),
    mMapEffects(),
    mNpcEffects(),
    mQuestLinks(),
    mNewQuestEffectId(paths.getIntValue("newQuestEffectId")),
    mCompleteQuestEffectId(paths.getIntValue("completeQuestEffectId")),
    mMap(nullptr)
{
    setWindowName("Quests");
    setResizable(true);
    setCloseButton(true);
    setStickyButtonLock(true);
    setSaveVisible(true);

    setDefaultSize(400, 350, ImagePosition::RIGHT);
    setMinWidth(310);
    setMinHeight(220);

    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    mQuestsListBox->setActionEventId("select");
    mQuestsListBox->addActionListener(this);

    mQuestScrollArea->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mText->setOpaque(false);
    mText->setLinkHandler(mItemLinkHandler);
    mTextScrollArea->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mQuestsListBox->setWidth(500);
    if (!gui || gui->getNpcFont()->getHeight() < 20)
        mQuestsListBox->setRowHeight(20);
    else
        mQuestsListBox->setRowHeight(gui->getNpcFont()->getHeight());

    ContainerPlacer placer;
    placer = getPlacer(0, 0);

    placer(0, 0, mQuestScrollArea, 4, 3).setPadding(3);
    placer(4, 0, mTextScrollArea, 4, 3).setPadding(3);
    placer(7, 3, mCloseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, LayoutType::SET);

    loadWindowState();
    enableVisibleSound(true);
    loadXmlFile(paths.getStringValue("questsFile"));
    loadXmlFile(paths.getStringValue("questsPatchFile"));
    loadXmlDir("questsPatchDir", loadXmlFile);
}

QuestsWindow::~QuestsWindow()
{
    delete2(mQuestsModel);

    for (std::map<int, std::vector<QuestItem*> >::iterator it
         = mQuests.begin(), it_end = mQuests.end(); it != it_end; ++ it)
    {
        std::vector<QuestItem*> &quests = (*it).second;
        for (std::vector<QuestItem*>::iterator it2 = quests.begin(),
             it2_end = quests.end(); it2 != it2_end; ++ it2)
        {
            delete *it2;
        }
    }
    delete_all(mAllEffects);
    mAllEffects.clear();

    delete2(mItemLinkHandler);
    mQuests.clear();
    mQuestLinks.clear();
    if (mCompleteIcon)
    {
        mCompleteIcon->decRef();
        mCompleteIcon = nullptr;
    }
    if (mIncompleteIcon)
    {
        mIncompleteIcon->decRef();
        mIncompleteIcon = nullptr;
    }
}

void QuestsWindow::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName, UseResman_true, SkipError_false);
    const XmlNodePtrConst root = doc.rootNode();
    if (!root)
        return;

    for_each_xml_child_node(varNode, root)
    {
        if (xmlNameEqual(varNode, "include"))
        {
            const std::string name = XML::getProperty(varNode, "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }
        else if (xmlNameEqual(varNode, "var"))
        {
            const int id = XML::getProperty(varNode, "id", 0);
            if (id < 0)
                continue;
            mVars[id] = 0;
            for_each_xml_child_node(questNode, varNode)
            {
                if (xmlNameEqual(questNode, "quest"))
                    loadQuest(id, questNode);
                else if (xmlNameEqual(questNode, "effect"))
                    loadEffect(id, questNode);
            }
        }
    }
}

void QuestsWindow::loadQuest(const int var, const XmlNodePtr node)
{
    if (!node)
        return;
    QuestItem *const quest = new QuestItem();
    // TRANSLATORS: quests window quest name
    quest->name = XML::langProperty(node, "name", _("unknown"));
    quest->group = XML::getProperty(node, "group", "");
    std::string incompleteStr = XML::getProperty(node, "incomplete", "");
    std::string completeStr = XML::getProperty(node, "complete", "");
    if (incompleteStr.empty() && completeStr.empty())
    {
        logger->log("complete flags incorrect");
        delete quest;
        return;
    }
    splitToIntSet(quest->incomplete, incompleteStr, ',');
    splitToIntSet(quest->complete, completeStr, ',');
    if (quest->incomplete.empty() && quest->complete.empty())
    {
        logger->log("complete flags incorrect");
        delete quest;
        return;
    }
    if (quest->incomplete.empty() || quest->complete.empty())
        quest->broken = true;

    for_each_xml_child_node(dataNode, node)
    {
        if (!xmlTypeEqual(dataNode, XML_ELEMENT_NODE))
            continue;
        const char *const data = reinterpret_cast<const char*>(
            XmlNodeGetContent(dataNode));
        if (!data)
            continue;
        std::string str = translator->getStr(data);

        if (xmlNameEqual(dataNode, "text"))
            quest->texts.push_back(QuestItemText(str, QuestType::TEXT));
        else if (xmlNameEqual(dataNode, "name"))
            quest->texts.push_back(QuestItemText(str, QuestType::NAME));
        else if (xmlNameEqual(dataNode, "reward"))
            quest->texts.push_back(QuestItemText(str, QuestType::REWARD));
    }
    mQuests[var].push_back(quest);
}

void QuestsWindow::loadEffect(const int var, const XmlNodePtr node)
{
    QuestEffect *const effect = new QuestEffect;
    effect->map = XML::getProperty(node, "map", "");
    effect->id = fromInt(XML::getProperty(node, "npc", -1), BeingTypeId);
    effect->effectId = XML::getProperty(node, "effect", -1);
    const std::string values = XML::getProperty(node, "value", "");
    splitToIntSet(effect->values, values, ',');

    if (effect->map.empty() || effect->id == BeingTypeId_negOne
        || effect->effectId == -1 || values.empty())
    {
        delete effect;
        return;
    }
    effect->var = var;
    mAllEffects.push_back(effect);
}

void QuestsWindow::action(const ActionEvent &event)
{
    const std::string &eventId = event.getId();
    if (eventId == "select")
    {
        const int id = mQuestsListBox->getSelected();
        if (id < 0)
            return;
        showQuest(mQuestLinks[id]);
    }
    else if (eventId == "close")
    {
        setVisible(Visible_false);
    }
}

void QuestsWindow::updateQuest(const int var, const int val)
{
    mVars[var] = val;
}

void QuestsWindow::rebuild(const bool playSound)
{
    mQuestsModel->clear();
    mQuestLinks.clear();
    StringVect &names = mQuestsModel->getNames();
    std::vector<Image*> &images = mQuestsModel->getImages();
    std::vector<QuestItem*> complete;
    std::vector<QuestItem*> incomplete;
    std::vector<QuestItem*> hidden;
    int updatedQuest = -1;
    int newCompleteStatus = -1;

    FOR_EACH (IntMapCIter, it, mVars)
    {
        const int var = (*it).first;
        const int val = (*it).second;
        const std::vector<QuestItem*> &quests = mQuests[var];
        FOR_EACH (std::vector<QuestItem*>::const_iterator, it2, quests)
        {
            if (!*it2)
                continue;
            QuestItem *const quest = *it2;
            // complete quest
            if (quest->complete.find(val) != quest->complete.end())
                complete.push_back(quest);
            // incomplete quest
            else if (quest->incomplete.find(val) != quest->incomplete.end())
                incomplete.push_back(quest);
            // hidden quest
            else
                hidden.push_back(quest);
        }
    }

    int k = 0;

    for (std::vector<QuestItem*>::const_iterator it = complete.begin(),
        it_end = complete.end(); it != it_end; ++ it, k ++)
    {
        QuestItem *const quest = *it;
        if (quest->completeFlag == 0 || (quest->broken
            && quest->completeFlag == -1))
        {
            updatedQuest = k;
            newCompleteStatus = 1;
        }
        quest->completeFlag = 1;
        mQuestLinks.push_back(quest);
        names.push_back(quest->name);
        if (mCompleteIcon)
        {
            mCompleteIcon->incRef();
            images.push_back(mCompleteIcon);
        }
        else
        {
            images.push_back(nullptr);
        }
    }

    for (std::vector<QuestItem*>::const_iterator it = incomplete.begin(),
        it_end = incomplete.end(); it != it_end; ++ it, k ++)
    {
        QuestItem *const quest = *it;
        if (quest->completeFlag == -1)
        {
            updatedQuest = k;
            newCompleteStatus = 0;
        }
        quest->completeFlag = 0;
        mQuestLinks.push_back(quest);
        names.push_back(quest->name);
        if (mIncompleteIcon)
        {
            mIncompleteIcon->incRef();
            images.push_back(mIncompleteIcon);
        }
        else
        {
            images.push_back(nullptr);
        }
    }

    FOR_EACH (std::vector<QuestItem*>::const_iterator, it, hidden)
        (*it)->completeFlag = -1;

    if (updatedQuest == -1 || updatedQuest >= static_cast<int>(
        mQuestLinks.size()))
    {
        updatedQuest = static_cast<int>(mQuestLinks.size() - 1);
    }
    if (updatedQuest >= 0)
    {
        mQuestsListBox->setSelected(updatedQuest);
        showQuest(mQuestLinks[updatedQuest]);
        if (playSound && effectManager)
        {
            switch (newCompleteStatus)
            {
                case 0:
                    effectManager->trigger(mNewQuestEffectId, localPlayer);
                    break;
                case 1:
                    effectManager->trigger(mCompleteQuestEffectId,
                        localPlayer);
                    break;
                default:
                    break;
            }
        }
    }
    updateEffects();
}

void QuestsWindow::showQuest(const QuestItem *const quest)
{
    if (!quest || !translator)
        return;

    const std::vector<QuestItemText> &texts = quest->texts;
    mText->clearRows();
    FOR_EACH (std::vector<QuestItemText>::const_iterator, it, texts)
    {
        const QuestItemText &data = *it;
        switch (data.type)
        {
            case QuestType::TEXT:
            case QuestType::REWARD:
            default:
                mText->addRow(translator->getStr(data.text));
                break;
            case QuestType::NAME:
                mText->addRow(std::string("[").append(translator->getStr(
                    data.text)).append("]"));
                break;
        }
    }
}

void QuestsWindow::setMap(const Map *const map)
{
    if (mMap != map)
    {
        mMap = map;
        mMapEffects.clear();
        if (!mMap)
            return;

        const std::string name = mMap->getProperty("shortName");
        FOR_EACH (std::vector<QuestEffect*>::const_iterator, it,  mAllEffects)
        {
            const QuestEffect *const effect = *it;
            if (effect && name == effect->map)
                mMapEffects.push_back(effect);
        }
        updateEffects();
    }
}

void QuestsWindow::updateEffects()
{
    NpcQuestEffectMap oldNpc = mNpcEffects;
    mNpcEffects.clear();

    FOR_EACH (std::vector<const QuestEffect*>::const_iterator,
              it,  mMapEffects)
    {
        const QuestEffect *const effect = *it;
        if (effect)
        {
            const IntMapCIter varIt = mVars.find(effect->var);
            if (varIt != mVars.end())
            {
                const std::set<int> &vals = effect->values;
                if (vals.find(mVars[effect->var]) != vals.end())
                    mNpcEffects[effect->id] = effect;
            }
        }
    }
    if (!actorManager)
        return;

    std::set<BeingTypeId> removeEffects;
    std::map<BeingTypeId, int> addEffects;

    // for old effects
    FOR_EACH (NpcQuestEffectMapCIter, it, oldNpc)
    {
        const BeingTypeId id = (*it).first;
        const QuestEffect *const effect = (*it).second;

        const NpcQuestEffectMapCIter itNew = mNpcEffects.find(id);
        if (itNew == mNpcEffects.end())
        {   // in new list no effect for this npc
            removeEffects.insert(id);
        }
        else
        {   // in new list exists effect for this npc
            const QuestEffect *const newEffect = (*itNew).second;
            if (effect != newEffect)
            { // new effects is not equal to old effect
                addEffects[id] = newEffect->effectId;
                removeEffects.insert(id);
            }
        }
    }

    // for new effects
    FOR_EACH (NpcQuestEffectMapCIter, it, mNpcEffects)
    {
        const BeingTypeId id = (*it).first;
        const QuestEffect *const effect = (*it).second;

        const NpcQuestEffectMapCIter itNew = oldNpc.find(id);
        // check if old effect was not present
        if (itNew == oldNpc.end())
            addEffects[id] = effect->effectId;
    }
    if (!removeEffects.empty() || !addEffects.empty())
        actorManager->updateEffects(addEffects, removeEffects);
}

void QuestsWindow::addEffect(Being *const being)
{
    if (!being)
        return;
    const BeingTypeId id = being->getSubType();
    const std::map<BeingTypeId, const QuestEffect*>::const_iterator
        it = mNpcEffects.find(id);
    if (it != mNpcEffects.end())
    {
        const QuestEffect *const effect = (*it).second;
        if (effect)
            being->addSpecialEffect(effect->effectId);
    }
}
