/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef RESOURCES_MAP_MAP_H
#define RESOURCES_MAP_MAP_H

#include "position.h"

#include "being/actor.h"

#include "enums/render/rendertype.h"

#include "enums/resources/map/blockmask.h"
#include "enums/resources/map/blocktype.h"
#include "enums/resources/map/maptype.h"

#include "resources/map/properties.h"

#include "listeners/configlistener.h"

#include <vector>

class AmbientLayer;
class MapHeights;
class MapItem;
class MapLayer;
class ObjectsLayer;
class Particle;
class Resource;
class SpecialLayer;
class Tileset;
class TileAnimation;
class WalkLayer;

struct MetaTile;

typedef std::vector<Tileset*> Tilesets;
typedef std::vector<MapLayer*> Layers;
typedef Layers::const_iterator LayersCIter;

typedef std::vector<AmbientLayer*> AmbientLayerVector;
typedef AmbientLayerVector::const_iterator AmbientLayerVectorCIter;
typedef AmbientLayerVector::iterator AmbientLayerVectorIter;

/**
 * A tile map.
 */
class Map final : public Properties, public ConfigListener
{
    public:
        enum CollisionTypes
        {
            COLLISION_EMPTY = 0,        // no collision
            COLLISION_WALL = 1,         // full collison
            COLLISION_AIR = 2,          // air units can walk
            COLLISION_WATER = 3,        // water units can walk
            COLLISION_GROUNDTOP = 4,    // no collision (chair, bed, etc)
            COLLISION_MAX = 5           // count index
        };

        /**
         * Constructor, taking map and tile size as parameters.
         */
        Map(const int width, const int height,
            const int tileWidth, const int tileHeight);

        A_DELETE_COPY(Map)

        /**
         * Destructor.
         */
        ~Map();

        /**
         * Initialize ambient layers. Has to be called after all the properties
         * are set.
         */
        void initializeAmbientLayers() restrict2;

        /**
         * Updates animations. Called as needed.
         */
        void update(const int ticks = 1) restrict2;

        /**
         * Draws the map to the given graphics output. This method draws all
         * layers, actors and overlay effects.
         *
         * TODO: For efficiency reasons, this method could take into account
         * the clipping rectangle set on the Graphics object. However,
         * currently the map is always drawn full-screen.
         */
        void draw(Graphics *restrict const graphics,
                  int scrollX,
                  int scrollY) restrict2 A_NONNULL(2);

        /**
         * Visualizes collision layer for debugging
         */
        void drawCollision(Graphics *restrict const graphics,
                           const int scrollX,
                           const int scrollY,
                           const MapTypeT drawFlags) const
                           restrict2 A_NONNULL(2);

        /**
         * Adds a layer to this map. The map takes ownership of the layer.
         */
        void addLayer(MapLayer *const layer) restrict2 A_NONNULL(2);

        /**
         * Adds a tileset to this map. The map takes ownership of the tileset.
         */
        void addTileset(Tileset *const tileset) restrict2 A_NONNULL(2);

        /**
         * Finds the tile set that a tile with the given global id is part of.
         */
        const Tileset *getTilesetWithGid(const int gid) const
                                         restrict2 A_WARN_UNUSED;

        /**
         * Get tile reference.
         */
        const MetaTile *getMetaTile(const int x,
                                    const int y) const
                                    restrict2 A_WARN_UNUSED;

        void addBlockMask(const int x, const int y,
                          const BlockTypeT type) restrict2;

        void setBlockMask(const int x, const int y,
                          const BlockTypeT type) restrict2;

        /**
         * Gets walkability for a tile with a blocking bitmask. When called
         * without walkmask, only blocks against colliding tiles.
         */
        bool getWalk(const int x, const int y,
                     const unsigned char blockWalkMask = BlockMask::WALL
                     | BlockMask::AIR | BlockMask::WATER) const
                     restrict2 A_WARN_UNUSED;

        void setWalk(const int x, const int y) restrict2;

        unsigned char getBlockMask(const int x,
                                   const int y) const restrict2;

        /**
         * Returns the width of this map in tiles.
         */
        int getWidth() const restrict2 noexcept A_WARN_UNUSED
        { return mWidth; }

        /**
         * Returns the height of this map in tiles.
         */
        int getHeight() const restrict2 noexcept A_WARN_UNUSED
        { return mHeight; }

        /**
         * Returns the tile width of this map.
         */
        int getTileWidth() const restrict2 noexcept A_WARN_UNUSED
        { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int getTileHeight() const restrict2 noexcept A_WARN_UNUSED
        { return mTileHeight; }

        const std::string getMusicFile() const restrict2 A_WARN_UNUSED;

        void setMusicFile(const std::string &restrict file) restrict2;

        const std::string getName() const restrict2 A_WARN_UNUSED;

        /**
         * Gives the map id based on filepath (ex: 009-1)
         */
        const std::string getFilename() const restrict2 A_WARN_UNUSED;

        const std::string getGatName() const restrict2 A_WARN_UNUSED;

        /**
         * Find a path from one location to the next.
         */
        Path findPath(const int startX, const int startY,
                      const int destX, const int destY,
                      const unsigned char blockWalkmask,
                      const int maxCost = 20) restrict2 A_WARN_UNUSED;

        /**
         * Adds a particle effect
         */
        void addParticleEffect(const std::string &restrict effectFile,
                               const int x,
                               const int y,
                               const int w = 0,
                               const int h = 0) restrict2;

        /**
         * Initializes all added particle effects
         */
        void initializeParticleEffects() const restrict2;

        /**
         * Adds a tile animation to the map
         */
        void addAnimation(const int gid,
                          TileAnimation *restrict const animation) restrict2
                          A_NONNULL(3);

        void setDrawLayersFlags(const MapTypeT &restrict n) restrict2;

        MapTypeT getDrawLayersFlags() const restrict2 A_WARN_UNUSED
        { return mDrawLayersFlags; }

        void addExtraLayer() restrict2;

        void saveExtraLayer() const restrict2;

        SpecialLayer *getTempLayer() const restrict2 noexcept A_WARN_UNUSED
        { return mTempLayer; }

        SpecialLayer *getSpecialLayer() const restrict2 noexcept A_WARN_UNUSED
        { return mSpecialLayer; }

        void setHasWarps(const bool n) restrict2 noexcept
        { mHasWarps = n; }

        bool getHasWarps() const restrict2 noexcept A_WARN_UNUSED
        { return mHasWarps; }

        std::string getUserMapDirectory() const restrict2 A_WARN_UNUSED;

        void addPortal(const std::string &restrict name,
                       const int type,
                       const int x, const int y,
                       const int dx, const int dy) restrict2;

        void addRange(const std::string &restrict name,
                      const int type,
                      const int x, const int y,
                      const int dx, const int dy) restrict2;

        void addPortalTile(const std::string &restrict name,
                           const int type,
                           const int x, const int y) restrict2;

        void updatePortalTile(const std::string &restrict name,
                              const int type,
                              const int x, const int y,
                              const bool addNew = true) restrict2;

        const std::vector<MapItem*> &getPortals() const restrict2 noexcept
                                                A_WARN_UNUSED
        { return mMapPortals; }

        /**
         * Gets the tile animation for a specific gid
         */
        const TileAnimation *getAnimationForGid(const int gid)
                                                const restrict2 A_WARN_UNUSED;

        void optionChanged(const std::string &restrict value)
                           restrict2 override final;

        MapItem *findPortalXY(const int x,
                              const int y) const restrict2 A_WARN_UNUSED;

        int getActorsCount() const restrict2 A_WARN_UNUSED
        { return CAST_S32(mActors.size()); }

        void setPvpMode(const int mode) restrict2;

        int getPvpMode() const restrict2 noexcept A_WARN_UNUSED
        { return mPvp; }

        const ObjectsLayer* getObjectsLayer() const restrict2 noexcept
                                            A_WARN_UNUSED
        { return mObjects; }

        std::string getObjectData(const unsigned x,
                                  const unsigned y,
                                  const int type) const
                                  restrict2 A_WARN_UNUSED;

        void indexTilesets() restrict2;

        void clearIndexedTilesets() restrict2;

        void setActorsFix(const int x, const int y) restrict2;

        int getVersion() const restrict2 noexcept A_WARN_UNUSED
        { return mVersion; }

        void setVersion(const int n) restrict2 noexcept
        { mVersion = n; }

        void reduce() restrict2;

        void redrawMap() restrict2 noexcept
        { mRedrawMap = true; }

        bool empty() const restrict2 A_WARN_UNUSED
        { return mLayers.empty(); }

        void setCustom(const bool b) restrict2 noexcept
        { mCustom = b; }

        bool isCustom() const restrict2 noexcept A_WARN_UNUSED
        { return mCustom; }

        const std::map<int, TileAnimation*> &getTileAnimations() const
                                            restrict2 noexcept A_WARN_UNUSED
        { return mTileAnimations; }

        void setAtlas(Resource *restrict const atlas) restrict2 noexcept
        { mAtlas = atlas; }

        const MetaTile *getMetaTiles() const restrict2 noexcept
        { return mMetaTiles; }

        const WalkLayer *getWalkLayer() const restrict2 noexcept
        { return mWalkLayer; }

        void setWalkLayer(WalkLayer *restrict const layer) restrict2 noexcept
        { mWalkLayer = layer; }

        void addHeights(const MapHeights *restrict const heights) restrict2
                        A_NONNULL(2);

        uint8_t getHeightOffset(const int x, const int y) const restrict2;

        void setMask(const int mask) restrict2;

        void updateDrawLayersList() restrict2;

        bool isHeightsPresent() const restrict2 noexcept
        { return mHeights != nullptr; }

        void updateConditionLayers() restrict2;

    protected:
        friend class Actor;
        friend class Minimap;

        /**
         * Adds an actor to the map.
         */
        Actors::iterator addActor(Actor *const actor) restrict2 A_NONNULL(2);

        /**
         * Removes an actor from the map.
         */
        void removeActor(const Actors::iterator &restrict iterator) restrict2;

    private:
        enum LayerType
        {
            FOREGROUND_LAYERS = 0,
            BACKGROUND_LAYERS
        };

        /**
         * Updates scrolling of ambient layers. Has to be called each game tick.
         */
        void updateAmbientLayers(const float scrollX,
                                 const float scrollY) restrict2;

        /**
         * Draws the foreground or background layers to the given graphics output.
         */
        void drawAmbientLayers(Graphics *restrict const graphics,
                               const LayerType type,
                               const int detail) const restrict2 A_NONNULL(2);

        /**
         * Tells whether the given coordinates fall within the map boundaries.
         */
        bool contains(const int x,
                      const int y) const restrict2 A_WARN_UNUSED;

        const int mWidth;
        const int mHeight;
        const int mTileWidth;
        const int mTileHeight;
        int mMaxTileHeight;
        MetaTile *const mMetaTiles;
        WalkLayer *mWalkLayer;
        Layers mLayers;
        Layers mDrawUnderLayers;
        Layers mDrawOverLayers;
        Tilesets mTilesets;
        Actors mActors;
        bool mHasWarps;

        // draw flags
        MapTypeT mDrawLayersFlags;

        // Pathfinding members
        unsigned int mOnClosedList;
        unsigned int mOnOpenList;

        // Overlay data
        AmbientLayerVector mBackgrounds;
        AmbientLayerVector mForegrounds;
        float mLastAScrollX;
        float mLastAScrollY;

        // Particle effect data
        struct ParticleEffectData final
        {
            ParticleEffectData(const std::string &restrict file0,
                               const int x0,
                               const int y0,
                               const int w0,
                               const int h0) noexcept :
                file(file0),
                x(x0),
                y(y0),
                w(w0),
                h(h0)
            {
            }

            const2 std::string file;
            const2 int x;
            const2 int y;
            const2 int w;
            const2 int h;
        };
        std::vector<ParticleEffectData> mParticleEffects;

        std::vector<MapItem*> mMapPortals;

        std::map<int, TileAnimation*> mTileAnimations;

        int mOverlayDetail;
        float mOpacity;
        const RenderType mOpenGL;
        int mPvp;
        bool mTilesetsIndexed;
        Tileset** mIndexedTilesets;
        int mIndexedTilesetsSize;
        int mActorFixX;
        int mActorFixY;
        int mVersion;

        SpecialLayer *mSpecialLayer;
        SpecialLayer *mTempLayer;
        ObjectsLayer *mObjects;
        MapLayer *mFringeLayer;

        int mLastX;
        int mLastY;
        int mLastScrollX;
        int mLastScrollY;

        int mDrawX;
        int mDrawY;
        int mDrawScrollX;
        int mDrawScrollY;
        int mMask;
        Resource *mAtlas;
        const MapHeights *mHeights;
        bool mRedrawMap;
        bool mBeingOpacity;
        bool mCachedDraw;
        bool mCustom;
        bool mDrawOnlyFringe;
};

#endif  // RESOURCES_MAP_MAP_H
