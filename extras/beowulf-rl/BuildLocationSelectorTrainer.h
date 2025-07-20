// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/BuildLocationSelector.h"

namespace beowulf {

class BuildLocationSelectorTrainer : public BuildLocationSelector
{
public:
    BuildLocationSelectorTrainer();
    ~BuildLocationSelectorTrainer();
};

} // namespace beowulf
