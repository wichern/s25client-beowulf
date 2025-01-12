// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "buildings/noBaseBuilding.h"

namespace s25py {

class PyBuilding
{
public:
    PyBuilding(const noBaseBuilding* building);

    // Destroy()
    // GetProductivity()

    BuildingType type;
    MapPoint pos;
    MapPoint flag_pos;

protected:
};

} // namespace s25py
