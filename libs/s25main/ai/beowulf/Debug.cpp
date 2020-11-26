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

#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Helper.h"

#include "ai/AIPlayer.h"
#include "RttrForeachPt.h"
#include "gameData/BuildingConsts.h"
#include "nodeObjs/noTree.h"
#include "nodeObjs/noFlag.h"
#include "buildings/nobUsual.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobMilitary.h"
#include "buildings/noBuildingSite.h"
#include "figures/nofPassiveSoldier.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>

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
    switch (dir.native_value())
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

AsciiMap::AsciiMap(const AIInterface& aii, unsigned short scale)
    : aii_(aii), offset_({0, 0})
{
    init(aii.gwb.GetSize(), scale);
}

AsciiMap::AsciiMap(
        const AIInterface& aii,
        const MapPoint& topLeft,
        const MapPoint& bottomRight,
        unsigned short scale)
    : aii_(aii)
{
    offset_ = topLeft;
    init(MapPoint(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y), scale);
}

AsciiMap::AsciiMap(
        const AIInterface& aii,
        const MapPoint& center,
        unsigned short radius,
        unsigned short scale)
    : aii_(aii)
{
    const MapExtent& size = aii.gwb.GetSize();
    offset_.x = std::max(center.x - radius, 0);
    offset_.y = std::max(center.y - radius, 0);
    unsigned short diameter = radius * 2;
    init(MapPoint(std::min(diameter, size.x), std::min(diameter, size.y)), scale);
}

AsciiMap::~AsciiMap()
{
    if (map_)
        delete [] map_;
}

void AsciiMap::draw(const MapPoint& pt, char c)
{
    if (pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if (pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), c);
}

void AsciiMap::draw(const MapPoint& pt, const std::string& str)
{
    if (pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if (pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), str);
}

void AsciiMap::drawRoad(const MapPoint& pt, unsigned char dir, bool fat)
{
    if (pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if (pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    AsciiPosition pos = getPos(ptS);

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

static const char* c_short_building_names[NUM_BUILDING_TYPES] = {
    "HQ", "Bar", "Gua", "", "Wat", "", "", "", "", "Fort", "GrM", "CoM", "IrM", "GoM", "Loo",
    "", "Cat", "Woo", "Fis", "Qua", "For", "Sla", "Hun", "Bre", "Arm", "Met",
    "Iro", "Cha", "Pig", "Sto", "", "Mil", "Bak", "Saw", "Min", "Wel",
    "Shi", "Far", "Don", "Har"
};

void AsciiMap::draw(const World& world, bool includeAnticipated)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        if (world.IsPlayerTerritory(pt, includeAnticipated))
            draw(pt, '_');
        else if (world.IsBorder(pt, includeAnticipated))
            draw(pt, 'b');

        if (world.HasFlag(pt))
            draw(pt, 'f');

        for (unsigned char dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
            if (world.HasRoad(pt, Direction(dir)))
                draw(pt, dir);
        }

        Building* building = world.GetBuilding(pt);
        if (building) {
            if (building->GetState() == Building::ConstructionRequested)
                draw(pt, std::string("(") + c_short_building_names[building->GetType()] + ")");
            else
                draw(pt, c_short_building_names[building->GetType()]);
            draw(pt, static_cast<unsigned char>(Direction::SOUTHEAST));
        }
    }
}

void AsciiMap::draw(const AIPlayer* player)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        const noFlag* flagObj = player->gwb.GetSpecObj<noFlag>(pt);
        if (flagObj && flagObj->GetPlayer() == player->GetPlayerId())
            draw(pt, 'f');

        for (const auto roadDir : helpers::EnumRange<RoadDir>{}) {
            if (PointRoad::Normal == player->gwb.GetRoad(pt, roadDir)) {
                drawRoad(pt, static_cast<unsigned char>(roadDir) + static_cast<unsigned char>(3));
            }
        }
    }

    const BuildingRegister& buildings = player->player.GetBuildingRegister();
    for(unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (nobUsual* building : buildings.GetBuildings(BuildingType(i)))
            draw(building->GetPos(), c_short_building_names[building->GetBuildingType()]);
    for(const nobBaseWarehouse* building : buildings.GetStorehouses())
        draw(building->GetPos(), c_short_building_names[building->GetBuildingType()]);
    for(const nobMilitary* building : buildings.GetMilitaryBuildings())
        draw(building->GetPos(), c_short_building_names[building->GetBuildingType()]);
    for(const noBuildingSite* building : buildings.GetBuildingSites())
        draw(building->GetPos(), std::string("(") + c_short_building_names[building->GetBuildingType()] + ")");
}

void AsciiMap::draw(const BuildLocations& buildLocations)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        drawBQ(pt, buildLocations.Get(pt));
    }
}

void AsciiMap::drawResources()
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        const MapNode& node = aii_.gwb.GetNode(pt);
        const Resource& res = node.resources;
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
        case Resource::Fish:
            draw(pt, "F" + std::to_string(res.getAmount()));
            break;
        case Resource::Water:
        default: break;
        }

        if (node.obj && node.obj->GetType() == NOP_GRAINFIELD)
            draw(pt, '#');

        DescIdx<TerrainDesc> t1 = aii_.gwb.GetNode(pt).t1;
        if (aii_.gwb.GetDescription().get(t1).Is(ETerrain::Walkable)) {
            NodalObjectType no = aii_.gwb.GetNO(pt)->GetType();

            if (no == NOP_TREE) {
                if (aii_.gwb.GetSpecObj<noTree>(pt)->ProducesWood())
                    draw(pt, "T");
            } else if (no == NOP_GRANITE) {
                draw(pt, "S");
            }
        }
    }
}

void AsciiMap::drawResourcesInReach(Resources& resources, BResourceType type)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        if (!aii_.IsOwnTerritory(pt))
            continue;
        draw(pt, std::to_string(resources.GetReachable(pt, type)));
    }
}

void AsciiMap::drawBorder(const World& world, bool includeAnticipated)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        if (world.IsBorder(pt, includeAnticipated)) {
            draw(pt, '!');
        }
    }
}

void AsciiMap::drawBorder(unsigned char player)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        if (aii_.gwb.GetNode(pt).boundary_stones[BorderStonePos::OnPoint] == (player + 1)) {
            draw(pt, '!');
        }
    }
}

void AsciiMap::drawBuildLocations(unsigned char player)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        drawBQ(pt, aii_.gwb.GetBQ(pt, player));
    }
}

void AsciiMap::drawBuildLocations(const World& world, bool includeAnticipated)
{
    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize())
    {
        drawBQ(pt, world.GetBQ(pt, includeAnticipated));
    }
}

void AsciiMap::drawAdditionalTerritory(const std::vector<MapPoint>& at)
{
    for (const MapPoint& pt : at)
    {
        draw(pt, "(_)");
    }
}

void AsciiMap::drawSoldiers(const AIPlayer* player)
{
    for (const nobMilitary* mil : player->getAIInterface().GetMilitaryBuildings()) {
        std::string str;
        for (const nofPassiveSoldier* soldier : mil->GetTroops()) {
            switch (soldier->GetRank()) {
            case 0: str += "p"; break;
            case 1: str += "P"; break;
            case 2: str += "S"; break;
            case 3: str += "O"; break;
            case 4: str += "G"; break;
            default: str += "?"; break;
            }
        }
        draw(mil->GetPos(), str);
    }
}

void AsciiMap::clear()
{
    // Fill with spaces.
    memset(map_, 0x20, map_buffer_len_ - 1);

    for (AsciiPosition::ElementType x = 0; x < w_; ++x) {
        // Add column number
        if (x % scale_w_ == 0)
            set({x + c_margin_left_, 1}, std::to_string((x / scale_w_) + offset_.x));
    }

    for (AsciiPosition::ElementType y = 0; y < h_; ++y) {
        // Add line number.
        if (y % scale_h_ == 0)
            set({1, y + c_margin_top_}, std::to_string((y / scale_h_) + offset_.y));

        // Add newline for every row.
        set({w_ - 1, y}, '\n');
    }

    // Add null terminator.
    map_[map_buffer_len_ - 1] = 0;

    RTTR_FOREACH_PT(MapPoint, aii_.gwb.GetSize()) {
        if (aii_.IsOwnTerritory(pt))
            draw(pt, '_');
        else
            draw(pt, '.');
    }
}

void AsciiMap::write(std::ostream& out) const
{
    assert(map_[map_buffer_len_ - 1] == 0); // Check for null terminator.
    out << map_ << std::flush;
}

void AsciiMap::init(const MapExtent& size, unsigned short scale)
{
    map_size_ = size;

    assert(scale > 0);
    scale_w_ = 2u + (2u * scale);
    scale_h_ = scale_w_ / 2;

    w_ = (size.x * scale_w_) + c_margin_left_ - scale + 1u; // +1 for '\n'
    h_ = (size.y * scale_h_) + c_margin_top_;

    map_buffer_len_ = static_cast<size_t>((w_ * h_) + 1); // +1 for null terminator
    map_ = new char[map_buffer_len_];

    clear();
}

AsciiMap::AsciiPosition AsciiMap::getPos(const MapPoint& pt) const
{
    AsciiPosition ret;
    ret.x = c_margin_left_;
    ret.x += static_cast<AsciiPosition::ElementType>(pt.x) * scale_w_;
    ret.x += ((pt.y + offset_.y) & 1) ? scale_h_ : 0; // offset on every second row
    ret.y = c_margin_top_ + (pt.y * scale_h_);
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

void AsciiMap::drawBQ(const MapPoint& pt, BuildingQuality bq)
{
    switch (bq) {
    case BQ_HUT:
        draw(pt, 'h');
        break;
    case BQ_HOUSE:
        draw(pt, 'H');
        break;
    case BQ_CASTLE:
        draw(pt, 'C');
        break;
    case BQ_MINE:
        draw(pt, 'm');
        break;
    case BQ_HARBOR:
        draw(pt, 'H');
        break;
    case BQ_FLAG:
    case BQ_NOTHING:
        // skip
        break;
    }
}

AsciiTable::AsciiTable(size_t columns)
    : columns_(columns)
{
    alignment_.resize(columns_);
    std::fill(alignment_.begin(), alignment_.end(), false);
}

void AsciiTable::addRow(const std::vector<std::string>& values)
{
    RTTR_Assert(values.size() == columns_);
    rows_.push_back(values);
}

void AsciiTable::alignLeft(int column, bool left)
{
    alignment_[column] = left;
}

void AsciiTable::writeHorizontal(std::ostream& out, const std::vector<size_t>& widths) const
{
    out << '+';
    for (size_t w : widths) {
        for (size_t i = 0; i < w; ++i)
            out << '-';
        out << '+';
    }
    out << '\n';
}

void AsciiTable::write(std::ostream& out) const
{
    // Determine the maximum width of every column.
    std::vector<size_t> widths;
    widths.resize(columns_);
    std::fill(widths.begin(), widths.end(), 0);

    for (const std::vector<std::string>& row : rows_)
        for (size_t i = 0; i < columns_; ++i)
            widths[i] = std::max(widths[i], row[i].size());


    writeHorizontal(out, widths);

    for (const std::vector<std::string>& row : rows_) {
        out << '|';
        for (size_t c = 0; c < columns_; ++c) {
            out << std::setw(widths[c]) << (alignment_.at(c) ? std::left : std::right) << row[c] << '|';
        }
        out << '\n';
        writeHorizontal(out, widths);
    }

    out << std::flush;
}

} // namespace beowulf
