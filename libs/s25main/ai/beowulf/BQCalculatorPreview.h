// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "world/BQCalculator.h"

class World;

namespace beowulf {

class BQCalculatorPreview : public BQCalculator
{
public:
    BQCalculatorPreview(const World& gw, const MapPoint& previewPt, BuildingQuality previewBq);
    virtual ~BQCalculatorPreview() = default;

private:
    BlockingManner GetBM(const MapPoint& pt) const override;

    const MapPoint& previewPt_;
    BuildingQuality previewBq_;
};

} // namespace beowulf