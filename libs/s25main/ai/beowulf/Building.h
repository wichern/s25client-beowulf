// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/Types.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/GoodTypes.h"
#include "gameTypes/JobTypes.h"
#include "gameTypes/Inventory.h"

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

    struct TrafficExpected {
        unsigned produced;
        unsigned consumed;
    };

private:
    World& world_;
    MapPoint pt_;
    BuildingType type_;
    State state_;
    unsigned group_;
    bool captured_;

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
    unsigned GetGroup() const;
    BuildingQuality GetQuality() const;
    unsigned GetDistance(const MapPoint& pt) const;
    const std::vector<BuildingType>& GetDestTypes(bool& checkGroup) const;
    bool GetCaptured() const { return captured_; }
    const TrafficExpected& GetTraffic() const;

    bool IsMilitary() const;
    bool IsWarehouse() const;

    unsigned GetGoods(GoodType good) const;
    unsigned GetJobs(Job job) const;

    // Whether this building is part of a production group.
    bool IsGrouped() const;

private:
    const Inventory* GetInventory() const;
};

} // namespace beowulf
