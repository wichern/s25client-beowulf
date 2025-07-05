// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config.h"
#include "types.h"
#include "gameTypes/MapCoordinates.h"
#include "gameTypes/JobTypes.h"
#include "gameTypes/GoodTypes.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/BuildingType.h"
#include "helpers/MaxEnumValue.h"
#include "EventManager.h"

#include <armadillo>

#include <array>

namespace beowulf {

class Environment;

class GameState
{
public:
    static constexpr size_t dimension = 23 // buildingTypeOffset
        + helpers::MaxEnumValue_v<BuildingType> + 1 - NUM_UNUSED_BLD_TYPES
        + helpers::MaxEnumValue_v<Job> + 1
        + helpers::MaxEnumValue_v<GoodType> + 1
        + 3 * (helpers::MaxEnumValue_v<BuildingType> + 1 - NUM_UNUSED_BLD_TYPES);

    //GameState();
    GameState(Environment* env = nullptr, const MapPoint& poi = MapPoint(), BuildingQuality bq = BuildingQuality::Nothing);

    GameState(const GameState& other) = default;
    GameState& operator=(const GameState& other);

    MapPoint GetHQPos() const;

    // Set POI before encoding the state, because it defines the visible area.
    void SetPOI(MapPoint const& poi) { poi_ = poi; }

    //inline const arma::colvec& Data() const { return data; }
    const arma::colvec& Encode() { RTTR_Assert(data.size() == dimension); return data; }

    void Update();

    bool isTerminal = false;
    MapPoint poi_;
    BuildingQuality bq_;

private:
    arma::colvec data;
    Environment* env_ = nullptr;

    double& trees;
    double& fish;
    double& granite;
    double& iron;
    double& coal;
    double& gold;
    double& granite_underground;
    double& player_territory;
    double& enemy_territory;
    double& visible_points;
    double& bq_near;
    double& bq_far;
    double& distance_to_border;
    double& distance_to_warehouse;
    double& enemy_catapults;
    double& water;
    const unsigned buildingTypeOffset;

};

} // namespace beowulf
