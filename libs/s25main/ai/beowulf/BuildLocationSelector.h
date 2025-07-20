// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "gameTypes/BuildingType.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/MapCoordinates.h"
#include "helpers/MaxEnumValue.h"

#include <mlpack/methods/ann/ffn.hpp>

#include <array>

class nobBaseWarehouse;
class GameWorldBase;
class AIInterface;

namespace beowulf {

class BuildLocationSelector
{
    using ffn_t = mlpack::FFN<mlpack::MeanSquaredError, mlpack::GaussianInitialization>;

    static constexpr size_t state_dim = 18;
    static constexpr unsigned building_type_count = helpers::MaxEnumValue_v<BuildingType> - NUM_UNUSED_BLD_TYPES;

public:
    BuildLocationSelector();
    ~BuildLocationSelector();

    /// @brief Select point to build the given building type
    MapPoint Select(AIInterface& aii, const nobBaseWarehouse* warehouse, BuildingType bld, bool random = false);

protected:
    std::array<ffn_t, building_type_count> networks_;

    arma::colvec CreateState(const GameWorldBase& gwb, const unsigned playerId, const MapPoint& pt, BuildingType bld, BuildingQuality bq);
};

} // namespace beowulf