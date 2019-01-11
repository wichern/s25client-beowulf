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
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Buildings.h"

#include "ai/AIInterface.h"
#include "gameTypes/BuildingQuality.h"
#include "gameData/MapConsts.h"
#include "gameData/BuildingConsts.h"
#include "world/MapGeometry.h"

#include <boost/nowide/fstream.hpp>
#include <boost/lexical_cast.hpp>

namespace beowulf {

static const std::string g_SvgDefs =
        "<defs>\n"
        "\t<g id=\"existing-building\">\n"
        "\t\t<circle r=\"12\" stroke=\"black\" fill=\"red\" />\n"
        "\t</g>\n"
        "\t<g id=\"flag\">\n"
        "\t\t<circle r=\"5\" stroke=\"black\" fill=\"yellow\" />\n"
        "\t</g>\n"
        "\t<g id=\"flag-planned\">\n"
        "\t\t<circle r=\"5\" stroke=\"black\" fill=\"grey\" />\n"
        "[...]"
        "\t<g id=\"road-e\">\n"
        "\t\t<line x1=\"0\" y1=\"0\" x2=\"56\" y2=\"0\" />\n"
        "\t</g>\n"
        "\t<g id=\"road-se\">\n"
        "\t\t<line x1=\"0\" y1=\"0\" x2=\"28\" y2=\"28\" />\n"
        "\t</g>\n"
        "\t<g id=\"road-sw\">\n"
        "\t\t<line x1=\"0\" y1=\"0\" x2=\"-28\" y2=\"28\" />\n"
        "\t</g>\n"
        "</defs>\n";

static const std::string BUILDING_ICONS[7] = {
    "",
    "flag",
    "hut",
    "house",
    "castle",
    "mine",
    "harbour"
};

static const std::string ROAD_DIRECTIONS[3] = {
    "e",
    "se",
    "sw"
};

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

std::string make_coords(unsigned x, unsigned y) {
    return "x=\"" + to_string(x) + "\" y=\"" + to_string(y) + "\"";
}

std::string make_text(unsigned x, unsigned y, const std::string& content) {
    return "<text " + make_coords(x, y) + " font-size=\"5px\">" + content + "</text>\n";
}

std::string make_building_quality(const Position& pt, BuildingQuality bq) {
    return "<use xlink:href=\"#" + BUILDING_ICONS[bq] + "\" " + make_coords(pt.x, pt.y) + "/>\n";
}

std::string make_existing_building(const Position& pt) {
    return "<use xlink:href=\"#existing-building\" " + make_coords(pt.x, pt.y) + "/>\n";
}

std::string make_road(const Position& pt, unsigned direction, const std::string& attributes = "")
{
    return "<use xlink:href=\"#road-" + ROAD_DIRECTIONS[direction] + "\" " + make_coords(pt.x, pt.y) + " " + attributes + " />\n";
}

//std::string make_road_label(const Position& pt, unsigned direction, const std::string& content)
//{
//    Position pos;
//    switch (direction) {
//    case Direction::WEST:
//        pos.x = pt.x - (56/2) - 5; // a little to the left
//        pos.y = pt.y + 5; // on top of that line
//        break;
//    case Direction::NORTHWEST:
//        pos.x = pt.x - (28/2) + 4; // alittle to the right
//        pos.y = pt.y - (28/2);
//        break;
//    case Direction::NORTHEAST:
//        pos.x = pt.x + (28/2) + 4; // alittle to the right
//        pos.y = pt.y - (28/2); // a little to the top
//        break;
//    default:
//        RTTR_Assert(false);
//    }
//    return "<text " + make_coords(pos.x, pos.y) + " font-size=\"4px\" color=\"blue\">" + content + "</text>\n";
//}

std::string make_planned_flag(const Position& pt)
{
    return "<use xlink:href=\"#flag-planned\" " + make_coords(pt.x, pt.y) + " />\n";
}

void CreateSvg(
        const AIInterface& aii,
        const BuildLocations& bl,
        const std::string& path)
{
    boost::nowide::ofstream stream;
    stream.open(path);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    stream << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\""
           << to_string(aii.gwb.GetWidth() * TR_W) << "\" height=\""
           << to_string(aii.gwb.GetHeight() * TR_H) << "px\">\n";
    stream << g_SvgDefs;

    for (MapPoint loc : bl.Get(BQ_HUT)) {
        Position pos = GetNodePos(loc);
        stream << make_building_quality(pos, bl.Get(loc));
        stream << make_text(pos.x, pos.y, to_string(loc.x) + ":" + to_string(loc.y));
    }

    stream << "</svg>\n";
}

void CreateSvg(
        const AIInterface& aii,
        const Buildings& buildings,
        const std::string& path)
{
    boost::nowide::ofstream stream;
    stream.open(path);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    stream << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\""
           << to_string(aii.gwb.GetWidth() * TR_W) << "\" height=\""
           << to_string(aii.gwb.GetHeight() * TR_H) << "px\">\n";
    stream << g_SvgDefs;

    for (const Building* b : buildings.Get()) {
        Position pos = GetNodePos(b->GetPos());
        stream << make_building_quality(pos, BUILDING_SIZE[b->GetType()]);
    }

    RTTR_FOREACH_PT(MapPoint, aii.gwb.GetSize())
    {
        Position pos = GetNodePos(pt);

        if (buildings.GetFlagState(pt) == FlagFinished)
            stream << make_building_quality(pos, BQ_FLAG);
        else if (buildings.GetFlagState(pt) == FlagRequested)
            stream << make_building_quality(pos, BQ_FLAG);

        for (unsigned dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
            if (buildings.GetRoadState(pt, Direction(dir)) == RoadFinished)
                stream << make_road(pos, dir - Direction::EAST, "stroke=\"black\"");
            if (buildings.GetRoadState(pt, Direction(dir)) == RoadRequested)
                stream << make_road(pos, dir - Direction::EAST, "stroke=\"yellow\"");
        }

        stream << make_text(pos.x, pos.y, to_string(pt.x) + ":" + to_string(pt.y));
    }

    stream << "</svg>\n";
}

} // namespace beowulf
