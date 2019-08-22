// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef BEOWULF_DEBUG_H_INCLUDED
#define BEOWULF_DEBUG_H_INCLUDED

#include "ai/AIInterface.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/Direction.h"

#include "ai/beowulf/World.h"
#include "ai/beowulf/RoadNetworks.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Resources.h"

#include <string>
#include <vector>

namespace beowulf {

std::string to_string(unsigned val);
std::string to_string(const MapPoint& pt);
std::string to_string(const std::vector<unsigned>& intvec);
std::string to_string(const unsigned* intvec, unsigned len);
std::string to_string(Direction dir);
std::string to_string(const std::vector<Direction>& route);

/**
 * Create an ASCII representation of the map (for debugging).
 */
class AsciiMap
{
public:
    /**
     * Create a new ASCII map with given size and scale.
     *
     * AsciiMap supports different scalings for the representation. The default is 1:
     *   .
     *  / \
     * .---.
     *  \ /
     *   .
     *
     * Every additional step increases the size of the output. scale = 2 becomes:
     *    .
     *   / \
     *  /   \
     * .-----.
     *  \   /
     *   \ /
     *    .
     *
     * @param size          Size of a map.
     * @param scale         Scaling of the representation.
     */
    AsciiMap(const AIInterface& aii, int scale = 1);
    ~AsciiMap();

    void draw(const MapPoint& pt, char c);
    void draw(const MapPoint& pt, const std::string& str);
    void draw(const MapPoint& pt, unsigned dir, bool fat = false);
    void draw(const World& world);
    void draw(const RoadNetworks& roadNetworks);
    void draw(const BuildLocations& buildLocations);
    void drawResources(const GameWorldBase& gwb);
    void drawResourcesInReach(Resources& resources, BResourceType type);
    void drawBuildLocations(const GameWorldBase& gwb, unsigned player);
    void drawBuildLocations(const World& world);

    void clear();
    void write(std::ostream& out = std::cout) const;

private:
    typedef Position AsciiPosition;
    static const size_t c_margin_left_ = 4;

    void init(const MapExtent& size, int scale);
    AsciiPosition getPos(const MapPoint& pt) const;
    size_t getIdx(const AsciiPosition& pos) const;
    void set(const AsciiPosition& pos, char c);
    void set(AsciiPosition pos, const std::string& str);
    bool onMap(const AsciiPosition& pos) const;

    void drawBQ(const MapPoint& pt, BuildingQuality bq);

    const AIInterface& aii_;
    MapExtent map_size_;
    AsciiPosition::ElementType w_;
    AsciiPosition::ElementType h_;
    AsciiPosition::ElementType scale_w_;
    AsciiPosition::ElementType scale_h_;
    size_t map_buffer_len_;
    char* map_;
};

} // namespace beowulf

#endif //! BEOWULF_DEBUG_H_INCLUDED
