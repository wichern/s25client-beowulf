// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "config.h"
#include "types.h"
#include "gameTypes/MapCoordinates.h"
#include "gameTypes/JobTypes.h"
#include "gameTypes/GoodTypes.h"
#include "helpers/MaxEnumValue.h"
#include "EventManager.h"

#include <armadillo>

#include <array>

namespace beowulf {

class Environment;

class GameState
{
public:
    static constexpr size_t point_count = (POI_RADIUS * POI_RADIUS + POI_RADIUS) * 3u + 1u;
    static constexpr size_t meta_information_count = 4 + helpers::MaxEnumValue_v<Job> + 1 + helpers::MaxEnumValue_v<GoodType> + 1;
    static constexpr size_t point_attribute_count = 10;
    static constexpr size_t dimension = meta_information_count + (point_count * point_attribute_count);

    GameState();
    GameState(Environment* env);

    GameState(const GameState& other) = default;
    GameState& operator=(const GameState& other) = default;

    MapPoint GetHQPos() const;

    // Set POI before encoding the state, because it defines the visible area.
    void SetPOI(MapPoint const& poi) { poi_ = poi; }

    //inline const arma::colvec& Data() const { return data; }
    const arma::colvec& Encode() { RTTR_Assert(data.size() == dimension); return data; }

    void Update();

    bool isTerminal = false;
    MapPoint poi_;

    // There are 4 parameters: action(0), param1(1), param2(2), param3(3).
    // Once the last parameter was selected, we create and send a game command and jump back to 0.
    unsigned actionStep_ = 0u;
    std::array<unsigned, 4> actionParams_;

private:
    arma::colvec data;
    Environment* env_ = nullptr;
};

} // namespace beowulf
