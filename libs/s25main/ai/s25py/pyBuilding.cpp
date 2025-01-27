// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyBuilding.h"

namespace s25py {

PyBuilding::PyBuilding(const noBaseBuilding* building)
    : type(building->GetBuildingType()), pos(building->GetPos()), flag_pos(building->GetFlagPos())
{}

} // namespace s25py