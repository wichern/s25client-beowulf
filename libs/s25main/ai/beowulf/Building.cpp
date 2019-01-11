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

#include "ai/beowulf/Building.h"
#include "ai/beowulf/Buildings.h"

#include "gameData/BuildingConsts.h"

#include <limits>

namespace beowulf {

Building::Building(
        Buildings& buildings,
        BuildingType type,
        State state)
    : buildings_(buildings),
      pos_(MapPoint::Invalid()),
      type_(type),
      state_(state),
      group_(InvalidProductionGroup)
{

}

const MapPoint& Building::GetPos() const
{
    return pos_;
}

MapPoint Building::GetFlag() const
{
    if (pos_.isValid())
        return buildings_.GetWorld().GetNeighbour(pos_, Direction::SOUTHEAST);
    return MapPoint::Invalid();
}

BuildingType Building::GetType() const
{
    return type_;
}

Building::State Building::GetState() const
{
    return state_;
}

ProductionGroup Building::GetGroup() const
{
    return group_;
}

BuildingQuality Building::GetQuality() const
{
    return BUILDING_SIZE[type_];
}

unsigned Building::GetDistance(const MapPoint& pos) const
{
    return buildings_.GetWorld().CalcDistance(GetPos(), pos);
}

bool Building::IsProduction() const
{
    return type_ == BLD_WOODCUTTER ||
           type_ == BLD_FORESTER ||
           type_ == BLD_SLAUGHTERHOUSE ||
           type_ == BLD_BREWERY ||
           type_ == BLD_ARMORY ||
           type_ == BLD_IRONSMELTER ||
           type_ == BLD_PIGFARM ||
           type_ == BLD_MILL ||
           type_ == BLD_BAKERY ||
           type_ == BLD_SAWMILL ||
           type_ == BLD_MINT ||
           type_ == BLD_WELL ||
           type_ == BLD_FARM ||
           type_ == BLD_DONKEYBREEDER;
}

} // namespace beowulf
