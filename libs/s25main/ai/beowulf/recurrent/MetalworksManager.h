// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/recurrent/RecurrentBase.h"

#include "gameTypes/GoodTypes.h"
#include "gameTypes/JobTypes.h"
#include "gameTypes/MapCoordinates.h"

#include <vector>
#include <queue>

class ToolNote;

namespace beowulf {

/*
 * The MetalworksManager receives tool requests and order an existing metalworks
 * building to build these tools.
 */
class MetalworksManager : public RecurrentBase
{
public:
    MetalworksManager(Beowulf* beowulf);

    void OnRun() override;

    void Request(GoodType type) { requests_.push(type); }
    unsigned GetRequestQueueLength() const { return requests_.size(); }
    bool JobOrToolOrQueueSpace(Job job, bool addMetalworksRequest = false, unsigned maxQueueLength = 20);

private:
    void OnToolNote(const ToolNote& note) override;

    void PlaceNextOrder();
    bool CheckMetalworksExists();
    void PlaceToolOrder(GoodType tool, int8_t count);

    std::queue<GoodType> requests_;
    MapPoint metalworksPt_;

    // Whether the metalworks has to finishe an ordered tool.
    bool isWorking_ = false;

};

} // namespace beowulf
