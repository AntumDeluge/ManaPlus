/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#include "particle.h"
#include "particlecontainer.h"

#include "debug.h"

ParticleContainer::ParticleContainer(ParticleContainer *parent,
                                     bool delParent):
    mDelParent(delParent),
    mNext(parent)
{}

ParticleContainer::~ParticleContainer()
{
    clearLocally();
    if (mDelParent)
    {
        delete mNext;
        mNext = nullptr;
    }
}

void ParticleContainer::clear()
{
    clearLocally();
    if (mNext)
        mNext->clear();
}

void ParticleContainer::moveTo(float x, float y)
{
    if (mNext)
        mNext->moveTo(x, y);
}

// -- particle list ----------------------------------------

ParticleList::ParticleList(ParticleContainer *parent, bool delParent):
    ParticleContainer(parent, delParent)
{}

ParticleList::~ParticleList()
{}

void ParticleList::addLocally(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mElements.push_back(particle);
    }
}

void ParticleList::removeLocally(Particle *particle)
{
    std::list<Particle *>::iterator it, it_end;
    for (it = mElements.begin(), it_end = mElements.end();
         it != it_end; )
    {
        if (*it == particle)
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ParticleList::clearLocally()
{
    for (std::list<Particle *>::const_iterator it = mElements.begin();
         it != mElements.end(); ++it)
    {
        (*it)->kill();
    }

    mElements.clear();
}

void ParticleList::moveTo(float x, float y)
{
    ParticleContainer::moveTo(x, y);

    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end(); )
    {
        (*it)->moveTo(x, y);
        if ((*it)->isExtinct())
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// -- particle vector ----------------------------------------

ParticleVector::ParticleVector(ParticleContainer *parent, bool delParent):
    ParticleContainer(parent, delParent)
{}

ParticleVector::~ParticleVector()
{}

void ParticleVector::setLocally(int index, Particle *particle)
{
    if (index < 0)
        return;

    delLocally(index);

    if (mIndexedElements.size() <= static_cast<unsigned>(index))
        mIndexedElements.resize(index + 1, nullptr);

    if (particle)
        particle->disableAutoDelete();
    mIndexedElements[index] = particle;
}

void ParticleVector::delLocally(int index)
{
    if (index < 0)
        return;

    if (mIndexedElements.size() <= static_cast<unsigned>(index))
        return;

    Particle *p = mIndexedElements[index];
    if (p)
    {
        mIndexedElements[index] = nullptr;
        p->kill();
    }
}

void ParticleVector::clearLocally()
{
    for (unsigned int i = 0; i < mIndexedElements.size(); i++)
        delLocally(i);
}

void ParticleVector::moveTo(float x, float y)
{
    ParticleContainer::moveTo(x, y);

    for (std::vector<Particle *>::iterator it = mIndexedElements.begin();
         it != mIndexedElements.end(); ++it)
    {
        if (*it)
        {
            (*it)->moveTo(x, y);

            if ((*it)->isExtinct())
            {
                (*it)->kill();
                *it = nullptr;
            }
        }
    }
}
