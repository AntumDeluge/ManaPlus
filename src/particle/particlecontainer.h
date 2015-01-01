/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#ifndef PARTICLE_PARTICLECONTAINER_H
#define PARTICLE_PARTICLECONTAINER_H

#include "localconsts.h"

/**
 * Set of particle effects.  May be stacked with other ParticleContainers.  All
 * operations herein affect such stacked containers, unless the operations end
 * in `Locally'.
 */
class ParticleContainer notfinal
{
    public:
        /**
         * Constructs a new particle container and assumes responsibility for
         * its parent (for all operations defined herein,
         * except when ending in `Locally')
         *
         * delParent means that the destructor should also free the parent.
         */
        explicit ParticleContainer(ParticleContainer *const parent = nullptr,
                                   const bool delParent = true);

        A_DELETE_COPY(ParticleContainer)

        virtual ~ParticleContainer();

        /**
         * Kills and removes all particle effects
         */
        void clear();

        /**
         * Kills and removes all particle effects (only in this container)
         */
        virtual void clearLocally()
        { }

        /**
         * Sets the positions of all elements
         */
        virtual void moveTo(const float x, const float y);

    protected:
        ParticleContainer *mNext;           /**< Contained container, if any */
        bool mDelParent;                    /**< Delete mNext in destructor */
};

#endif  // PARTICLE_PARTICLECONTAINER_H
