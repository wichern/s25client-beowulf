// Copyright (c) 2005 - 2019 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef BEOWULF_RECURRENT_METALWORKSMANAGER_H_INCLUDED
#define BEOWULF_RECURRENT_METALWORKSMANAGER_H_INCLUDED

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

#endif //! BEOWULF_RECURRENT_METALWORKSMANAGER_H_INCLUDED
