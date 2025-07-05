// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifndef POI_RADIUS
#define POI_RADIUS 14u    // 14 is the maximum range of a catapult
#endif

//#define DEBUG_OUTPUT

#include "types.h"
#include "helpers/EnumArray.h"
#include "s25util/warningSuppression.h"
#include "gameTypes/GoodTypes.h"

#include <vector>

namespace beowulf {

constexpr helpers::EnumArray<double, GoodType> SUPPRESS_UNUSED GOOD_GAIN_REWARD = {{
    /* Beer */              0.1,
    /* Tongs */             0.0,
    /* Hammer */            0.0,
    /* Axe */               0.0,
    /* Saw */               0.0,
    /* PickAxe */           0.0,
    /* Shovel */            0.0,
    /* Crucible */          0.0,
    /* RodAndLine */        0.0,
    /* Scythe */            0.0,
    /* WaterEmpty */        0.0,
    /* Water */             0.0,
    /* Cleaver */           0.0,
    /* Rollingpin */        0.0,
    /* Bow */               0.0,
    /* Boat */              0.0,
    /* Sword */             0.1,
    /* Iron */              0.05,
    /* Flour */             0.01,
    /* Fish */              0.02,
    /* Bread */             0.02,
    /* ShieldRomans */      0.1,
    /* Wood */              0.005,
    /* Boards */            0.02,
    /* Stones */            0.02,
    /* ShieldVikings */     0.1,
    /* ShieldAfricans */    0.1,
    /* Grain */             0.001,
    /* Coins */             0.2,
    /* Gold */              0.05,
    /* IronOre */           0.03,
    /* Coal */              0.04,
    /* Meat */              0.01,
    /* Ham */               0.02,
    /* ShieldJapanese */    0.1,
    /* Grapes */            0.0,
    /* Wine */              0.0
}};

}  // namespace beowulf
