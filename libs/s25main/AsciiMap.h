// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "BuildingRegister.h"
#include "GamePlayer.h"
#include "RttrForeachPt.h"
#include "buildings/noBuildingSite.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHarborBuilding.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobUsual.h"
#include "helpers/EnumArray.h"
#include "helpers/MaxEnumValue.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noFlag.h"
#include "gameData/BuildingProperties.h"
#include <iomanip>
#include <iostream>

/**
 * Create an ASCII representation of the map (for debugging).
 *
 * This is a header-only class, so that you can include it when needed for debugging.
 * It will not be part of the binary otherwise.
 */
class AsciiMap
{
    const GameWorldBase& gwb_;
    unsigned short scale_ = 1;

public:
    enum class Border : uint8_t
    {
        Locations = 0,
        ASCII
    };

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
    explicit AsciiMap(const GameWorldBase& gwb, unsigned short scale = 1);
    explicit AsciiMap(const GameWorldBase& gwb, const MapPoint& center, unsigned short radius, unsigned short scale = 1, Border border = Border::Locations);
    explicit AsciiMap(const GameWorldBase& gwb, const MapPoint& center, unsigned short width, unsigned short height, unsigned short scale = 1, Border border = Border::Locations);

    ~AsciiMap();

    void draw(const MapPoint& pt, char c);
    void draw(const MapPoint& pt, const std::string& str);
    void drawRoad(const MapPoint& pt, RoadDir dir, bool fat = false);
    void drawPlayer(unsigned playerId);

    void drawBq(const MapPoint& pt, BuildingQuality bq);
    void drawDStar(const noRoadNode* goal);

    void clear();
    void write(std::ostream& out = std::cout) const;

    using AsciiPosition = Position;
    AsciiPosition::ElementType w_;
    AsciiPosition::ElementType h_;
private:

    void init(const MapExtent& size);

    int xMargin() const;
    int yMargin() const;

    AsciiPosition getPos(const MapPoint& pt) const;
    size_t getIdx(const AsciiPosition& pos) const;
    void set(const AsciiPosition& pos, char c);
    void set(AsciiPosition pos, const std::string& str);
    bool onMap(const AsciiPosition& pos) const;

    MapExtent map_size_;
    MapPoint offset_;
    AsciiPosition::ElementType scale_w_;
    AsciiPosition::ElementType scale_h_;
    size_t map_buffer_len_;
    char* map_;
    Border border_ = Border::Locations;
};

constexpr helpers::EnumArray<const char*, BuildingType> SHORT_BLD_NAMES = {
  {"HQ",  "Bar", "Gua", "",    "Wat",  "Vin",  "Win",    "Tem", "",  "Fort", "GrM", "CoM", "IrM", "GoM",
   "Loo", "",    "Cat", "Woo", "Fis", "Qua", "For", "Sla", "Hun", "Bre",  "Arm", "Met", "Iro", "Cha",
   "Pig", "Sto", "",    "Mil", "Bak", "Saw", "Min", "Wel", "Shi", "Far",  "Don", "Har"}};

inline AsciiMap::AsciiMap(const GameWorldBase& gwb, unsigned short scale) : gwb_(gwb), scale_(scale), offset_({0, 0})
{
    init(gwb_.GetSize());
}

inline AsciiMap::AsciiMap(const GameWorldBase& gwb,
        const MapPoint& center,
        unsigned short radius,
        unsigned short scale,
        Border border)
: gwb_(gwb), scale_(scale), border_(border)
{
    const MapExtent& size = gwb_.GetSize();
    offset_.x = std::max(center.x - radius, 0);
    offset_.y = std::max(center.y - radius, 0);
    unsigned short diameter = radius * 2;
    init(MapPoint(std::min(diameter, size.x), std::min(diameter, size.y)));
}

inline AsciiMap::AsciiMap(const GameWorldBase& gwb,
        const MapPoint& center,
        unsigned short width,
        unsigned short height,
        unsigned short scale,
        Border border)
: gwb_(gwb), scale_(scale), border_(border)
{
    const MapExtent& size = gwb_.GetSize();
    offset_.x = std::max(center.x - width/2, 0);
    offset_.y = std::max(center.y - height/2, 0);
    init(MapPoint(std::min(width, size.x), std::min(height, size.y)));
}

inline AsciiMap::~AsciiMap()
{
    delete[] map_;
}

inline void AsciiMap::draw(const MapPoint& pt, char c)
{
    if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), c);
}

inline void AsciiMap::draw(const MapPoint& pt, const std::string& str)
{
    if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), str);
}

inline void AsciiMap::drawRoad(const MapPoint& pt, RoadDir dir, bool fat)
{
    if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    AsciiPosition pos = getPos(ptS);

    AsciiPosition::ElementType length = scale_w_ - 1;
    if(dir != RoadDir::East)
        length = (scale_w_ / 2) - 1;

    switch(dir)
    {
        case RoadDir::East:
        {
            pos.x += 1;
        }
        break;
        case RoadDir::SouthEast:
        {
            pos.x += 1;
            pos.y += 1;
        }
        break;
        case RoadDir::SouthWest:
        {
            pos.x -= 1;
            pos.y += 1;
        }
        break;
    }

    for(AsciiPosition::ElementType i = 0; i < length && onMap(pos); ++i)
    {
        switch(dir)
        {
            case RoadDir::East:
            {
                set(pos, fat ? '=' : '-');
                pos.x += 1;
            }
            break;
            case RoadDir::SouthEast:
            {
                set(pos, '\\');
                if(fat)
                    set({pos.x + 1, pos.y}, '\\');
                pos.x += 1;
                pos.y += 1;
            }
            break;
            case RoadDir::SouthWest:
            {
                set(pos, '/');
                if(fat)
                    set({pos.x + 1, pos.y}, '/');
                pos.x -= 1;
                pos.y += 1;
            }
            break;
        }
    }
}

inline void AsciiMap::drawPlayer(unsigned playerId)
{
    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
    {
        const auto& node = gwb_.GetNode(pt);
        if(node.owner == (playerId + 1) && node.obj == nullptr)
            draw(pt, '.');
    }

    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
    {
        const auto* flagObj = gwb_.GetSpecObj<noFlag>(pt);
        if(flagObj && flagObj->GetPlayer() == playerId)
            draw(pt, 'f');

        for(const auto roadDir : helpers::EnumRange<RoadDir>{})
        {
            PointRoad type = gwb_.GetRoad(pt, roadDir);
            if(PointRoad::Normal == type || PointRoad::Donkey == type)
            {
                drawRoad(pt, roadDir);
            }
        }
    }

    const GamePlayer& player = gwb_.GetPlayer(playerId);
    const BuildingRegister& buildings = player.GetBuildingRegister();

    for(const auto bldType : helpers::enumRange<BuildingType>())
    {
        if(BuildingProperties::IsUsual(bldType))
            for(nobUsual* bld : buildings.GetBuildings(bldType))
                draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);
    }

    for(nobBaseWarehouse* bld : buildings.GetStorehouses())
        draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);

    for(nobMilitary* bld : buildings.GetMilitaryBuildings())
        draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);

    for(nobHarborBuilding* bld : buildings.GetHarbors())
        draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);

    for(const noBuildingSite* building : buildings.GetBuildingSites())
        draw(building->GetPos(), std::string("(") + SHORT_BLD_NAMES[building->GetBuildingType()] + ")");
}

inline void AsciiMap::drawDStar(const noRoadNode* goal)
{
    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
    {
        const auto* roadNode = gwb_.GetSpecObj<noRoadNode>(pt);
        if(roadNode) {
            std::string g{"?"};
            std::string rhs{"?"};
            if (roadNode->dstar.Exists(goal)) {
                const auto& nodeData = roadNode->dstar.Get(goal);
                g = nodeData.g == std::numeric_limits<unsigned>::max() ? std::string("~") : std::to_string(nodeData.g);
                rhs = nodeData.rhs == std::numeric_limits<unsigned>::max() ? std::string("~") : std::to_string(nodeData.rhs);
            }
            draw(roadNode->GetPos(), g + std::string(",") + rhs);
        }
    }
}

inline void AsciiMap::clear()
{
    // Fill with spaces.
    memset(map_, ' ', map_buffer_len_ - 1);

    switch (border_)
    {
    case Border::Locations:
    {
        // Add column number
        for(AsciiPosition::ElementType x = 0; x < w_; ++x)
            if(x % scale_w_ == 0)
                set({x + xMargin(), 1}, std::to_string((x / scale_w_) + offset_.x));

        // Add line number.
        for(AsciiPosition::ElementType y = 0; y < h_; ++y)
            if(y % scale_h_ == 0)
                set({1, y + yMargin()}, std::to_string((y / scale_h_) + offset_.y));
    } break;
    case Border::ASCII:
    {
        set({0, h_ - 1}, '+');
        set({w_ - 2, h_ - 1}, '+');
        for(AsciiPosition::ElementType x = 1; x < w_ - 2; ++x)
            set({x, h_ - 1}, '-');
        for(AsciiPosition::ElementType y = 0; y < h_ - 1; ++y) {
            set({0, y}, '|');
            set({w_ - 2, y}, '|');
        }
    } break;
    default: break;
    }

    // Add newline for every row.
    for(AsciiPosition::ElementType y = 0; y < h_; ++y)
        set({w_ - 1, y}, '\n');

    // Add null terminator.
    map_[map_buffer_len_ - 1] = 0;

    // Fill map with terrain data
    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize()) {
        if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
            continue;
        if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
            continue;
            
        if(gwb_.IsWaterPoint(pt))
            draw(pt, '~');
        else if(const auto* obj = gwb_.GetNode(pt).obj) {
            if (obj->GetType() == NodalObjectType::Tree)
                draw(pt, 't');
            else if (obj->GetType() == NodalObjectType::Granite)
                draw(pt, '^');
            else if (obj->GetType() == NodalObjectType::Grainfield)
                draw(pt, '#');
        }
    }
}

inline void AsciiMap::init(const MapExtent& size)
{
    map_size_ = size;

    scale_w_ = 2u + (2u * scale_);
    scale_h_ = scale_w_ / 2;

    w_ = (map_size_.x * scale_w_) + xMargin() - scale_ + 1u; // +1 for '\n'
    h_ = (map_size_.y * scale_h_) + yMargin();

    map_buffer_len_ = static_cast<size_t>((w_ * h_) + 1U); // +1 for null terminator
    map_ = new char[map_buffer_len_];

    clear();
}

inline AsciiMap::AsciiPosition AsciiMap::getPos(const MapPoint& pt) const
{
    AsciiPosition ret;
    ret.x = xMargin();
    ret.x += static_cast<AsciiPosition::ElementType>(pt.x) * scale_w_;
    ret.x += ((pt.y + offset_.y) & 1) ? scale_h_ : 0U; // offset on every second row
    ret.y = yMargin() + (pt.y * scale_h_);
    return ret;
}

inline size_t AsciiMap::getIdx(const AsciiPosition& pos) const
{
    return static_cast<size_t>(pos.y * w_ + pos.x);
}

inline void AsciiMap::set(const AsciiPosition& pos, char c)
{
    size_t idx = getIdx(pos);
    assert(idx < (map_buffer_len_ - 1U)); // bounds check
    map_[idx] = c;
}

inline void AsciiMap::set(AsciiMap::AsciiPosition pos, const std::string& str)
{
    for(std::string::size_type i = 0; i < str.length() && onMap(pos); ++i)
    {
        set(pos, str[i]);
        pos.x++;
    }
}
inline bool AsciiMap::onMap(const AsciiPosition& pos) const
{
    return pos.x >= 0 && (pos.x + 1) < w_ && pos.y >= 0 && pos.y < h_;
}

inline void AsciiMap::write(std::ostream& out) const
{
    assert(map_[map_buffer_len_ - 1] == 0); // Check for null terminator.
    out << map_ << std::flush;
}

inline void AsciiMap::drawBq(const MapPoint& pt, BuildingQuality bq)
{
    switch (bq) {
    case BuildingQuality::Hut:
        draw(pt, 'h');
        break;
    case BuildingQuality::House:
        draw(pt, 'H');
        break;
    case BuildingQuality::Castle:
        draw(pt, 'C');
        break;
    case BuildingQuality::Mine:
        draw(pt, 'm');
        break;
    case BuildingQuality::Harbor:
        draw(pt, 'H');
        break;
    default:
        break;
    }
}

inline int AsciiMap::xMargin() const
{
    switch (border_)
    {
    case Border::Locations:
    {
        return 4;
    } break;
    case Border::ASCII:
    {
        return 1;
    } break;
    default: break;
    }

    return 0;
}

inline int AsciiMap::yMargin() const
{
    switch (border_)
    {
    case Border::Locations:
    {
        return 2;
    } break;
    case Border::ASCII:
    {
        return 0;
    } break;
    default: break;
    }

    return 0;
}
