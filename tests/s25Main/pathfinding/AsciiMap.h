// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "BuildingRegister.h"
#include "GamePlayer.h"
#include "RttrForeachPt.h"
#include "buildings/noBuildingSite.h"
#include "buildings/nobBaseWarehouse.h"
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
    AsciiMap(const GameWorldBase& gwb, unsigned short scale = 1);

    ~AsciiMap();

    void draw(const MapPoint& pt, char c);
    void draw(const MapPoint& pt, const std::string& str);
    void drawRoad(const MapPoint& pt, RoadDir dir, bool fat = false);
    void draw(const World& world, bool includeAnticipated);
    void drawPlayer(unsigned playerId);

    void clear();
    void write(std::ostream& out = std::cout) const;

private:
    typedef Position AsciiPosition;
    static const unsigned char c_margin_left_ = 4;
    static const unsigned char c_margin_top_ = 2;

    void init(const MapExtent& size);

    AsciiPosition getPos(const MapPoint& pt) const;
    size_t getIdx(const AsciiPosition& pos) const;
    void set(const AsciiPosition& pos, char c);
    void set(AsciiPosition pos, const std::string& str);
    bool onMap(const AsciiPosition& pos) const;

    // void drawBQ(const MapPoint& pt, BuildingQuality bq);

    MapExtent map_size_;
    MapPoint offset_;
    AsciiPosition::ElementType w_;
    AsciiPosition::ElementType h_;
    AsciiPosition::ElementType scale_w_;
    AsciiPosition::ElementType scale_h_;
    size_t map_buffer_len_;
    char* map_;
};

constexpr helpers::EnumArray<const char*, BuildingType> SHORT_BLD_NAMES = {
  {"HQ",  "Bar", "Gua", "",    "Wat", "",    "",    "",    "",    "Fort", "GrM", "CoM", "IrM", "GoM",
   "Loo", "",    "Cat", "Woo", "Fis", "Qua", "For", "Sla", "Hun", "Bre",  "Arm", "Met", "Iro", "Cha",
   "Pig", "Sto", "",    "Mil", "Bak", "Saw", "Min", "Wel", "Shi", "Far",  "Don", "Har"}};

AsciiMap::AsciiMap(const GameWorldBase& gwb, unsigned short scale) : gwb_(gwb), scale_(scale), offset_({0, 0})
{
    init(gwb_.GetSize());
}

AsciiMap::~AsciiMap()
{
    if(map_)
        delete[] map_;
}

void AsciiMap::draw(const MapPoint& pt, char c)
{
    if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), c);
}

void AsciiMap::draw(const MapPoint& pt, const std::string& str)
{
    if(pt.x < offset_.x || pt.x - offset_.x >= map_size_.x)
        return;
    if(pt.y < offset_.y || pt.y - offset_.y >= map_size_.y)
        return;
    MapPoint ptS = MapPoint(pt.x - offset_.x, pt.y - offset_.y);
    set(getPos(ptS), str);
}

void AsciiMap::drawRoad(const MapPoint& pt, RoadDir dir, bool fat)
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

void AsciiMap::drawPlayer(unsigned playerId)
{
    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
    {
        if(gwb_.GetNode(pt).owner == (playerId + 1))
            draw(pt, '.');
    }

    RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
    {
        const noFlag* flagObj = gwb_.GetSpecObj<noFlag>(pt);
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
        if(!BuildingProperties::IsUsual(bldType))
            continue;
        for(nobUsual* bld : buildings.GetBuildings(bldType))
            draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);
    }

    for(nobBaseWarehouse* bld : buildings.GetStorehouses())
        draw(bld->GetPos(), SHORT_BLD_NAMES[bld->GetBuildingType()]);

    for(const noBuildingSite* building : buildings.GetBuildingSites())
        draw(building->GetPos(), std::string("(") + SHORT_BLD_NAMES[building->GetBuildingType()] + ")");
}

void AsciiMap::clear()
{
    // Fill with spaces.
    memset(map_, ' ', map_buffer_len_ - 1);

    for(AsciiPosition::ElementType x = 0; x < w_; ++x)
    {
        // Add column number
        if(x % scale_w_ == 0)
            set({x + c_margin_left_, 1}, std::to_string((x / scale_w_) + offset_.x));
    }

    for(AsciiPosition::ElementType y = 0; y < h_; ++y)
    {
        // Add line number.
        if(y % scale_h_ == 0)
            set({1, y + c_margin_top_}, std::to_string((y / scale_h_) + offset_.y));

        // Add newline for every row.
        set({w_ - 1, y}, '\n');
    }

    // Add null terminator.
    map_[map_buffer_len_ - 1] = 0;
}

void AsciiMap::init(const MapExtent& size)
{
    map_size_ = size;

    scale_w_ = 2u + (2u * scale_);
    scale_h_ = scale_w_ / 2;

    w_ = (size.x * scale_w_) + c_margin_left_ - scale_ + 1u; // +1 for '\n'
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
    for(std::string::size_type i = 0; i < str.length() && onMap(pos); ++i)
    {
        set(pos, str[i]);
        pos.x++;
    }
}
bool AsciiMap::onMap(const AsciiPosition& pos) const
{
    return pos.x >= 0 && (pos.x + 1) < w_ && pos.y >= 0 && pos.y < h_;
}

void AsciiMap::write(std::ostream& out) const
{
    assert(map_[map_buffer_len_ - 1] == 0); // Check for null terminator.
    out << map_ << std::flush;
}
