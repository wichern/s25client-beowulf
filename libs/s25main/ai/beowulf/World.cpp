// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/World.h"

#include "world/BQCalculator.h"
#include "world/GameWorldBase.h"

namespace beowulf {

World::World(GameWorldBase& gwb)
 : gwb_(gwb)
{

}

World::~World()
{

}

} // namespace beowulf