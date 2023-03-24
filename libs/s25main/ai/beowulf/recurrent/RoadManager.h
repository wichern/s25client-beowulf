// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

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
