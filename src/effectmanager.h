/*
 *  The ManaPlus Client
 *  Copyright (C) 2008  Fate <fate.tmw@googlemail.com>
 *  Copyright (C) 2008  Chuck Miller <shadowmil@gmail.com>
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

#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "enums/resources/spritedirection.h"

#include "particle/particletimer.h"

#include "resources/effectdescription.h"

#include <vector>
#include <list>

#include "localconsts.h"

class Being;
class Particle;

class EffectManager final
{
    public:
        EffectManager();

        A_DELETE_COPY(EffectManager)

        ~EffectManager();

        void loadXmlFile(const std::string &fileName);

        /**
         * Triggers a effect with the id, at
         * the specified being.
         */
        bool trigger(const int id,
                     Being *const being,
                     const int rotation = 0);

        bool triggerDirection(const int id,
                              Being *const being,
                              const SpriteDirection::Type &direction);

        Particle *triggerReturn(const int id,
                                Being *const being,
                                const int rotation = 0);

        /**
         * Triggers a effect with the id, at
         * the specified x and y coordinate.
         */
        bool trigger(const int id,
                     const int x, const int y,
                     const int endTime,
                     const int rotation = 0);

        void triggerDefault(int effectId,
                            Being *const being,
                            const int defaultEffectId);

        void triggerDefault(int effectId,
                            const int x,
                            const int y,
                            const int endTime,
                            const int defaultEffectId);

        void logic();

        void clear();

    private:
        std::vector<EffectDescription> mEffects;
        std::list<ParticleTimer> mTimers;
};

extern EffectManager *effectManager;

#endif  // EFFECTMANAGER_H
