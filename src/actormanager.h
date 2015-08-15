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

#ifndef ACTORMANAGER_H
#define ACTORMANAGER_H

#include "flooritem.h"

#include "enums/simpletypes/allowsort.h"
#include "enums/simpletypes/allplayers.h"
#include "enums/simpletypes/beingtypeid.h"
#include "enums/simpletypes/npcnames.h"

#include "listeners/configlistener.h"

#include "utils/stringmap.h"
#include "utils/stringvector.h"

#include "localconsts.h"

class Being;
class LocalPlayer;
class Map;

struct ChatObject;

typedef std::set<ActorSprite*> ActorSprites;
typedef ActorSprites::iterator ActorSpritesIterator;
typedef ActorSprites::const_iterator ActorSpritesConstIterator;

class ActorManager final: public ConfigListener
{
    public:
        ActorManager();

        A_DELETE_COPY(ActorManager)

        ~ActorManager();

        /**
         * Sets the map on which ActorSprites are created.
         */
        void setMap(Map *const map);

        /**
         * Sets the current player.
         */
        void setPlayer(LocalPlayer *const player) A_NONNULL(2);

        /**
         * Create a Being and add it to the list of ActorSprites.
         */
        Being *createBeing(const BeingId id,
                           const ActorTypeT type,
                           const BeingTypeId subtype) A_WARN_UNUSED;

        static Being *cloneBeing(const Being *const srcBeing,
                                 const int dx, const int dy,
                                 const int id);

        /**
         * Create a FloorItem and add it to the list of ActorSprites.
         */
        FloorItem *createItem(const BeingId id,
                              const int itemId,
                              const int x, const int y,
                              const int amount,
                              const ItemColor color,
                              const int subX, const int subY);

        /**
         * Destroys the given ActorSprite at the end of
         * ActorManager::logic.
         */
        void destroy(ActorSprite *const actor);

        void erase(ActorSprite *const actor);

        void undelete(const ActorSprite *const actor);

        /**
         * Returns a specific Being, by id;
         */
        Being *findBeing(const BeingId id) const A_WARN_UNUSED;

        /**
         * Returns a being at specific coordinates.
         */
        Being *findBeing(const int x, const int y, const ActorTypeT
                         type = ActorType::Unknown) const A_WARN_UNUSED;

        /**
         * Returns a being at the specific pixel.
         */
        Being *findBeingByPixel(const int x, const int y,
                                const AllPlayers allPlayers = AllPlayers_false)
                                const A_WARN_UNUSED;

        /**
         * Returns a beings at the specific pixel.
         */
        void findBeingsByPixel(std::vector<ActorSprite*> &beings,
                               const int x, const int y,
                               const AllPlayers allPlayers) const;

        /**
         * Returns a portal at the specific tile.
         */
        Being *findPortalByTile(const int x, const int y) const A_WARN_UNUSED;

        /**
         * Returns a specific FloorItem, by id.
         */
        FloorItem *findItem(const BeingId id) const A_WARN_UNUSED;

        /**
         * Returns a FloorItem at specific coordinates.
         */
        FloorItem *findItem(const int x, const int y) const A_WARN_UNUSED;

        /**
         * Returns a being nearest to specific coordinates.
         *
         * @param x           X coordinate in pixels.
         * @param y           Y coordinate in pixels.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         */
        Being *findNearestLivingBeing(const int x, const int y,
                                      int maxTileDist,
                                      const ActorTypeT type,
                                      const Being *const excluded)
                                      const A_WARN_UNUSED;

        /**
         * Returns a being nearest to another being.
         *
         * @param aroundBeing The being to search around.
         * @param maxTileDist Maximal distance in tiles. If minimal distance is
         *                    larger, no being is returned.
         * @param type        The type of being to look for.
         */
        Being *findNearestLivingBeing(const Being *const aroundBeing,
                                      const int maxTileDist,
                                      const ActorTypeT type,
                                      const AllowSort allowSort)
                                      const A_WARN_UNUSED;

        /**
         * Finds a being by name and (optionally) by type.
         */
        Being *findBeingByName(const std::string &name,
                               const ActorTypeT
                               type = ActorType::Unknown)
                               const A_WARN_UNUSED;

       /**
        * Finds a nearest being by name and (optionally) by type.
        */
        Being *findNearestByName(const std::string &name,
                                 const ActorTypeT &type
                                 = ActorType::Unknown) const A_WARN_UNUSED;

       /**
        * Finds most damaged player, non-enemy and alive
        * \param maxTileDist maximal distance.
        */
        Being *findMostDamagedPlayer(const int maxTileDist)
                                     const A_WARN_UNUSED;

       /**
        * Heal all players in distance.
        *
        * \param maxdist maximal distance. If minimal distance is larger,
        *                no being is returned
        */
//        void HealAllTargets(Being *aroundBeing, int maxdist,
//                                            ActorTypeT type) const;

        void healTarget() const;

        void heal(const Being *const target) const;

        void itenplz() const;

        /**
         * Returns the whole list of beings.
         */
        const ActorSprites &getAll() const;

        /**
         * Returns true if the given ActorSprite is in the manager's list,
         * false otherwise.
         *
         * \param actor the ActorSprite to search for
         */
        bool hasActorSprite(const ActorSprite *const actor)
                            const A_WARN_UNUSED;

        /**
         * Performs ActorSprite logic and deletes ActorSprite scheduled to be
         * deleted.
         */
        void logic();

        /**
         * Destroys all ActorSprites except the local player
         */
        void clear();

        void addBlock(const BeingId id);

        void deleteBlock(const BeingId id);

        bool isBlocked(const BeingId id) const;

        void printAllToChat() const;

        static void printBeingsToChat(const ActorSprites &beings,
                                      const std::string &header);

        static void printBeingsToChat(const std::vector<Being*> &beings,
                                      const std::string &header);

        void getPlayerNames(StringVect &names,
                            const NpcNames npcNames) const;

        void getMobNames(StringVect &names) const;

        void updatePlayerNames() const;

        void updatePlayerColors() const;

        void updatePlayerGuild() const;

#ifdef TMWA_SUPPORT
        void parseLevels(std::string levels) const;
#endif

        bool pickUpAll(const int x1, const int y1, const int x2, const int y2,
                       const bool serverBuggy = false);

        bool pickUpNearest(const int x, const int y, int maxdist) const;

        void optionChanged(const std::string &name) override final;

        void removeAttackMob(const std::string &name);

        void removePickupItem(const std::string &name);

        void addPriorityAttackMob(const std::string &name);

        void addAttackMob(const std::string &name);

        void addIgnoreAttackMob(const std::string &name);

        void addPickupItem(const std::string &name);

        void addIgnorePickupItem(const std::string &name);

        void setPriorityAttackMobs(const std::list<std::string> &mobs)
        { mPriorityAttackMobs = mobs; }

        void setAttackMobs(const std::list<std::string> &mobs)
        { mAttackMobs = mobs; }

        int getPriorityAttackMobsSize() const A_WARN_UNUSED
        { return static_cast<int>(mPriorityAttackMobs.size()); }

        int getAttackMobsSize() const A_WARN_UNUSED
        { return static_cast<int>(mAttackMobs.size()); }

        int getPickupItemsSize() const A_WARN_UNUSED
        { return static_cast<int>(mPickupItems.size()); }

#define defList(list1, mob) \
        bool isIn##list1##List(const std::string &name) const A_WARN_UNUSED\
        { return m##list1##mob##Set.find(name) != m##list1##mob##Set.end(); }\
        void rebuild##list1##mob();\
        std::set<std::string> get##list1##mob##Set() const A_WARN_UNUSED\
        { return m##list1##mob##Set; }\
        std::list<std::string> get##list1##mob() const A_WARN_UNUSED\
        { return m##list1##mob; }

        defList(Attack, Mobs)
        defList(PriorityAttack, Mobs)
        defList(IgnoreAttack, Mobs)
        defList(Pickup, Items)
        defList(IgnorePickup, Items)

        const StringIntMap &getAttackMobsMap() const A_WARN_UNUSED
        { return mAttackMobsMap; }

        const StringIntMap &getPriorityAttackMobsMap() const A_WARN_UNUSED
        { return mPriorityAttackMobsMap; }

        int getAttackMobIndex(const std::string &name) const A_WARN_UNUSED;

        int getPriorityAttackMobIndex(const std::string &name)
                                      const A_WARN_UNUSED;

        int getPickupItemIndex(const std::string &name) const A_WARN_UNUSED;

        static int getIndexByName(const std::string &name,
                                  const StringIntMap &map)
                                  A_WARN_UNUSED;

        bool checkForPickup(const FloorItem *const item) const A_WARN_UNUSED;

        void updateEffects(const std::map<BeingTypeId, int> &addEffects,
                           const std::set<BeingTypeId> &removeEffects) const;

        void updateBadges();

#ifdef EATHENA_SUPPORT
        void removeRoom(const int chatId);

        void updateRoom(const ChatObject *const newChat);
#endif

    protected:
        bool validateBeing(const Being *const aroundBeing,
                           Being *const being,
                           const ActorTypeT &type,
                           const Being *const excluded = nullptr,
                           const int maxCost = 20) const A_WARN_UNUSED;

        Being *findNearestLivingBeing(const Being *const aroundBeing,
                                      const int maxdist,
                                      const ActorTypeT &type,
                                      const int x, const int y,
                                      const Being *const excluded,
                                      const AllowSort allowSort)
                                      const A_WARN_UNUSED;

        void loadAttackList();

        void storeAttackList() const;

        ActorSprites mActors;
        ActorSprites mDeleteActors;
        std::set<BeingId> mBlockedBeings;
        Map *mMap;
        std::string mSpellHeal1;
        std::string mSpellHeal2;
        std::string mSpellItenplz;
        bool mTargetDeadPlayers;
        bool mTargetOnlyReachable;
        bool mCyclePlayers;
        bool mCycleMonsters;
        bool mCycleNPC;
        bool mExtMouseTargeting;

#define defVarsP(mob) \
        std::list<std::string> mPriority##mob;\
        std::set<std::string> mPriority##mob##Set;\
        StringIntMap mPriority##mob##Map;

#define defVars(mob) \
        std::list<std::string> m##mob;\
        std::set<std::string> m##mob##Set;\
        StringIntMap m##mob##Map;\
        std::list<std::string> mIgnore##mob;\
        std::set<std::string> mIgnore##mob##Set;

        defVarsP(AttackMobs)
        defVars(AttackMobs)
        defVars(PickupItems)
};

extern ActorManager *actorManager;

#endif  // ACTORMANAGER_H
