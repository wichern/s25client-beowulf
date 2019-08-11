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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/Debug.h"
#include "gameData/BuildingConsts.h"
#include "nodeObjs/noTree.h"

#include <boost/lexical_cast.hpp>

namespace beowulf {

std::string to_string(unsigned val) {
    return boost::lexical_cast<std::string>(val);
}

std::string to_string(const MapPoint& pt)
{
    return "(" + to_string(pt.x) + ":" + to_string(pt.y) + ")";
}

std::string to_string(const std::vector<unsigned>& intvec)
{
    std::string ret("(");

    for (unsigned i = 0; i < intvec.size(); ++i) {
        ret += to_string(intvec[i]);
        if (i+1 != intvec.size())
            ret += ", ";
    }

    return ret + ")";
}

std::string to_string(const std::vector<Direction>& route)
{
    std::string ret("(");

    for (unsigned i = 0; i < route.size(); ++i) {
        ret += to_string(route[i]);
        if (i+1 != route.size())
            ret += ", ";
    }

    return ret + ")";
}

std::string to_string(const unsigned* intvec, unsigned len)
{
    std::string ret("(");

    for (unsigned i = 0; i < len; ++i) {
        ret += to_string(intvec[i]);
        if (i+1 != len)
            ret += ", ";
    }

    return ret + ")";
}

std::string to_string(Direction dir)
{
    switch (dir.toUInt())
    {
    case Direction::SOUTHEAST:
        return "SE";
    case Direction::SOUTHWEST:
        return "SW";
    case Direction::WEST:
        return "W";
    case Direction::EAST:
        return "E";
    case Direction::NORTHEAST:
        return "NE";
    case Direction::NORTHWEST:
        return "NW";
    default:
        return "<unknown>";
    }
}

AsciiMap::AsciiMap(const AIInterface& aii, int scale)
    : aii_(aii)
{
    init(aii.gwb.GetSize(), scale);
}

AsciiMap::~AsciiMap()
{
    if (map_)
        delete [] map_;
}

void AsciiMap::draw(const MapPoint& pt, char c)
{
    set(getPos(pt), c);
}

void AsciiMap::draw(const MapPoint& pt, const std::string& str)
{
    set(getPos(pt), str);
}

void AsciiMap::draw(const MapPoint& pt, unsigned dir, bool fat)
{
    AsciiPosition pos = getPos(pt);

    AsciiPosition::ElementType length = scale_w_ - 1;
    if (dir != Direction::WEST && dir != Direction::EAST)
        length = (scale_w_/2) - 1;

    switch (dir)
    {
    case Direction::EAST:
    {
        pos.x += 1;
    } break;
    case Direction::SOUTHEAST:
    {
        pos.x += 1;
        pos.y += 1;
    } break;
    case Direction::SOUTHWEST:
    {
        pos.x -= 1;
        pos.y += 1;
    } break;
    }

    for (AsciiPosition::ElementType i = 0; i < length && onMap(pos); ++i) {
        switch (dir)
        {
        case Direction::EAST:
        {
            set(pos, fat ? '=' : '-');
            pos.x += 1;
        } break;
        case Direction::SOUTHEAST:
        {
            set(pos, '\\');
            if (fat) set({pos.x + 1, pos.y}, '\\');
            pos.x += 1;
            pos.y += 1;
        } break;
        case Direction::SOUTHWEST:
        {
            set(pos, '/');
            if (fat) set({pos.x + 1, pos.y}, '/');
            pos.x -= 1;
            pos.y += 1;
        } break;
        }
    }
}

void AsciiMap::draw(const Buildings& buildings)
{
    static const char* c_short_building_names[NUM_BUILDING_TYPES] = {
        "HQ", "Bar", "Gua", "", "Wat", "", "", "", "", "Fort", "GrM", "CoM", "IrM", "GoM", "Loo",
        "", "Cat", "Woo", "Fis", "Qua", "For", "Sla", "Hun", "Bre", "Arm", "Met",
        "Iro", "Cha", "Pig", "Sto", "", "Mil", "Bak", "Saw", "Min", "Wel",
        "Shi", "Far", "Don", "Har"
    };

    RTTR_FOREACH_PT(MapPoint, map_size_)
    {
        if (buildings.HasFlag(pt))
            draw(pt, 'f');

        for (unsigned dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
            if (buildings.HasRoad(pt, Direction(dir)))
                draw(pt, dir);
        }

        Building* building = buildings.Get(pt);
        if (building) {
            draw(pt, c_short_building_names[building->GetType()]);
        }
    }
}

void AsciiMap::draw(const RoadNetworks& roadNetworks)
{
    RTTR_FOREACH_PT(MapPoint, map_size_)
    {
        rnet_id_t id = roadNetworks.Get(pt);
        if (id != InvalidRoadNetwork)
            draw(pt, std::to_string(id));
    }
}

void AsciiMap::drawResources(const GameWorldBase& world)
{
    RTTR_FOREACH_PT(MapPoint, map_size_)
    {
        const Resource& res = world.GetNode(pt).resources;
        switch (res.getType()) {
        case Resource::Iron:
            draw(pt, "I" + std::to_string(res.getAmount()));
            break;
        case Resource::Gold:
            draw(pt, "G" + std::to_string(res.getAmount()));
            break;
        case Resource::Coal:
            draw(pt, "C" + std::to_string(res.getAmount()));
            break;
        case Resource::Granite:
            draw(pt, "Gr" + std::to_string(res.getAmount()));
            break;
        case Resource::Water:
            draw(pt, "W" + std::to_string(res.getAmount()));
            break;
        case Resource::Fish:
            draw(pt, "F" + std::to_string(res.getAmount()));
            break;
        default: break;
        }

        DescIdx<TerrainDesc> t1 = world.GetNode(pt).t1;
        if (world.GetDescription().get(t1).Is(ETerrain::Walkable)) {
            NodalObjectType no = world.GetNO(pt)->GetType();

            if (no == NOP_TREE) {
                if (world.GetSpecObj<noTree>(pt)->ProducesWood())
                    draw(pt, "T");
            } else if (no == NOP_GRANITE) {
                draw(pt, "S");
            }
        }
    }
}

void AsciiMap::clear()
{
    // Fill with spaces.
    memset(map_, 0x20, map_buffer_len_ - 1);

    for (AsciiPosition::ElementType y = 0; y < h_; ++y) {
        // Add line number.
        if (y % scale_h_ == 0)
            set({1, y}, std::to_string(y / scale_h_));

        // Add newline for every row.
        set({w_ - 1, y}, '\n');
    }

    // Add null terminator.
    map_[map_buffer_len_ - 1] = 0;

    RTTR_FOREACH_PT(MapPoint, map_size_) {
        if (aii_.IsOwnTerritory(pt))
            draw(pt, '_');
        else
            draw(pt, '.');
    }
}

void AsciiMap::write(std::ostream& out) const
{
    assert(map_[map_buffer_len_ - 1] == 0); // Check for null terminator.
    out << map_;
}

void AsciiMap::init(const MapExtent& size, int scale)
{
    map_size_ = size;

    assert(scale > 0);
    scale_w_ = static_cast<AsciiPosition::ElementType>(2 + 2*scale);
    scale_h_ = scale_w_ / 2;

    w_ = (size.x * scale_w_) + c_margin_left_ - scale + 1; // +1 for '\n'
    h_ = size.y * scale_h_;

    map_buffer_len_ = (w_ * h_) + 1; // +1 for null terminator
    map_ = new char[map_buffer_len_];

    clear();
}

AsciiMap::AsciiPosition AsciiMap::getPos(const MapPoint& pt) const
{
    AsciiPosition ret;
    ret.x = c_margin_left_;
    ret.x += static_cast<AsciiPosition::ElementType>(pt.x) * scale_w_;
    ret.x += (pt.y & 1) ? scale_h_ : 0; // offset on every second row
    ret.y = pt.y * scale_h_;
    return ret;
}

size_t AsciiMap::getIdx(const AsciiPosition& pos) const
{
    return static_cast<size_t>(pos.y * w_ + pos.x);
}

void AsciiMap::set(const AsciiPosition& pos, char c)
{
    size_t idx = getIdx(pos);
    assert(idx < (map_buffer_len_ - 1)); // bounds check
    map_[idx] = c;
}

void AsciiMap::set(AsciiMap::AsciiPosition pos, const std::string& str)
{
    for (std::string::size_type i = 0; i < str.length() && onMap(pos); ++i) {
        set(pos, str[i]);
        pos.x++;
    }
}

bool AsciiMap::onMap(const AsciiPosition& pos) const
{
    return pos.x >= 0 && (pos.x + 1) < w_ &&
            pos.y >= 0 && pos.y < h_;
}

} // namespace beowulf
