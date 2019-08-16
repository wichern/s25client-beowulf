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
#ifndef BEOWULF_BUILDING_H_INCLUDED
#define BEOWULF_BUILDING_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/BuildingQuality.h"

#include <vector>

namespace beowulf {

class World;

/**
 * @brief The Building class contains the additional data Beowulf needs to store.
 */
class Building
{
public:
    enum State {
        // The planning of this building is requested.
        PlanningRequest,

        // A game command for placing this building has been sent with AIInterface.
        ConstructionRequested,

        // A construction site has been placed.
        UnderConstruction,

        // The builing is fully built.
        Finished,

        // A game command for destruction of this building has been sent with AIInterface.
        DestructionRequested
    };

private:
    World& world_;
    MapPoint pt_;
    BuildingType type_;
    State state_;
    pgroup_id_t group_;

    // Only 'Buildings' can create and destroy building objects or change their state or group.
    friend class World;
    friend class BuildingsPlan;

    Building(World& world, BuildingType type, State state);
    ~Building() = default;

public:
    const MapPoint& GetPt() const;
    MapPoint GetFlag() const;
    BuildingType GetType() const;
    State GetState() const;
    pgroup_id_t GetGroup() const;
    BuildingQuality GetQuality() const;
    unsigned GetDistance(const MapPoint& pt) const;
    const std::vector<BuildingType>& GetDestTypes(bool& checkGroup) const;

    /**
     * Whether this building will produce goods.
     */
    bool IsProduction() const;
    bool IsStorage() const;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDING_H_INCLUDED
