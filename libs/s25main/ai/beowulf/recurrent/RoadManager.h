// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef BEOWULF_RECURRENT_ROADMANAGER_H_INCLUDED
#define BEOWULF_RECURRENT_ROADMANAGER_H_INCLUDED

#include "ai/beowulf/recurrent/RecurrentBase.h"

#include "world/NodeMapBase.h"

#include <vector>
#include <set>

namespace beowulf {

class Building;
class BuildLocations;

class RoadManager : public RecurrentBase
{
public:
    RoadManager(Beowulf* beowulf);

    void OnRun() override;

    // Can optionally update a buildLocations object.
    bool Connect(const Building* building, BuildLocations* buildLocations = nullptr);
    void ConnectUnconnected();
    void RemoveUnused();

    bool IsConnected(const MapPoint& src, const MapPoint& dst) const;

private:
    void OnBuildingNote(const BuildingNote& note) override;
    void OnRoadNote(const RoadNote& note) override;

    void SetUsage(const Building* building, const std::vector<Direction>& route);
    void SetUsage(const Building* building, const MapPoint& pt, Direction dir);
    void UnsetUsage(const Building* building);
    unsigned GetTraffic(const MapPoint& pt, Direction dir, unsigned char d) const;
    std::vector<const Building*>& GetUsers(const MapPoint& pt, Direction dir);

    std::set<const Building*> connected;

    struct Node {
        std::vector<const Building*> users[3];
        bool isFarmLand = false;
        unsigned usage[3][2] = { { 0 } }; // Usage is directional (0 ... from here to neighbour, 1 ... from neighbour to here)
    };
    NodeMapBase<Node> nodes_;
};

} // namespace beowulf

#endif //! BEOWULF_RECURRENT_ROADMANAGER_H_INCLUDED
