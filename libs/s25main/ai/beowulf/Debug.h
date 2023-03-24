// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIInterface.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/Direction.h"

#include "ai/beowulf/World.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Resources.h"

#include <string>
#include <vector>

class AIPlayer;

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
    AsciiMap(const AIInterface& aii, unsigned short scale = 1);
    AsciiMap(const AIInterface& aii, const MapPoint& center, unsigned short radius, unsigned short scale = 1);
    AsciiMap(const AIInterface& aii, const MapPoint& topLeft, const MapPoint& bottomRight, unsigned short scale = 1);
    ~AsciiMap();

    void draw(const MapPoint& pt, char c);
    void draw(const MapPoint& pt, const std::string& str);
    void drawRoad(const MapPoint& pt, Direction dir, bool fat = false);
    void draw(const World& world, bool includeAnticipated);
    void draw(const AIPlayer* player);
    void draw(const BuildLocations& buildLocations);
    void drawResources();
    void drawResourcesInReach(Resources& resources, BResourceType type);
    void drawBorder(const World& world, bool includeAnticipated);
    void drawBorder(unsigned char player);
    void drawBuildLocations(unsigned char player);
    void drawBuildLocations(const World& world, bool includeAnticipated);
    void drawAdditionalTerritory(const std::vector<MapPoint>& at);
    void drawSoldiers(const AIPlayer* player);

    void clear();
    void write(std::ostream& out = std::cout) const;

private:
    typedef Position AsciiPosition;
    static const unsigned char c_margin_left_ = 4;
    static const unsigned char c_margin_top_ = 2;

    void init(const MapExtent& size, unsigned short scale);
    AsciiPosition getPos(const MapPoint& pt) const;
    size_t getIdx(const AsciiPosition& pos) const;
    void set(const AsciiPosition& pos, char c);
    void set(AsciiPosition pos, const std::string& str);
    bool onMap(const AsciiPosition& pos) const;

    void drawBQ(const MapPoint& pt, BuildingQuality bq);

    const AIInterface& aii_;
    MapExtent map_size_;
    MapPoint offset_;
    AsciiPosition::ElementType w_;
    AsciiPosition::ElementType h_;
    AsciiPosition::ElementType scale_w_;
    AsciiPosition::ElementType scale_h_;
    size_t map_buffer_len_;
    char* map_;
};


/*
 * @todo: Min column width
 * @todo: set_cell with overloads for different types and a precision argument.
 * @todo: add_header and markdown style
 */
class AsciiTable
{
public:
    AsciiTable(size_t columns);

    void addRow(const std::vector<std::string>& values);
    void alignLeft(int column, bool left = true);

    void write(std::ostream& out = std::cout) const;

private:
    size_t columns_;

    void writeHorizontal(std::ostream& out, const std::vector<size_t>& widths) const;

    std::vector<bool> alignment_;
    std::vector<std::vector<std::string>> rows_;
};

} // namespace beowulf
