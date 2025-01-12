// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "RoadSegment.h"
#include "helpers/EnumArray.h"
#include "noCoordBase.h"
#include "gameTypes/Direction.h"
#include "gameTypes/RoadPathDirection.h"

class Ware;
class SerializedGameData;

// Basisklasse für Gebäude und Flagge (alles, was als "Straßenknoten" dient
class noRoadNode : public noCoordBase
{
protected:
    unsigned char player;

private:
    helpers::EnumArray<RoadSegment*, Direction> routes;

public:
    // For Pathfinding
    // cost from start
    mutable unsigned cost; //-V730_NOINIT
    // distance to target
    mutable unsigned targetDistance; //-V730_NOINIT
    // estimated total distance (cost + distance)
    mutable unsigned estimate; //-V730_NOINIT
    mutable unsigned last_visit;
    mutable const noRoadNode* prev; //-V730_NOINIT
    /// Direction to previous node, includes SHIP_DIR
    mutable RoadPathDirection dir_; //-V730_NOINIT

    // Datastructure for D*-lite
    struct DStarNode
    {
        // Destination for which the g and lhs values are-
        MapPoint dest;
        unsigned g;   // estimate of the distance to dest
        unsigned rhs; // one step lookahead of g (g + edgeCost(this, neighbour))
        unsigned k_m;
        unsigned visit_gf; // required for LRU removal
        struct Key
        {
            unsigned k1;
            unsigned k2;
            bool operator<(const Key& other)
            {
                if(k1 < other.k1)
                    return true;
                if(k1 == other.k2)
                    return k2 < other.k2;
                return false;
            }
        } k;

        inline void CalculateKey(unsigned h)
        {
            // @todo: h is constant. we could buffer the value or calculate it at the beginning
            //        Also: h is the same for all nodes in this RoadNode. Check how expensive it is to calculate.
            k.k2 = std::min(g, rhs);
            k.k1 = k.k2 + h + k_m;
        }

        inline bool locally_consistend() { return g == rhs; }
    };
    mutable std::vector<DStarNode> dstar_;

    noRoadNode(NodalObjectType nop, MapPoint pos, unsigned char player);
    noRoadNode(SerializedGameData& sgd, unsigned obj_id);
    ~noRoadNode() override;

    void Destroy() override;
    void Serialize(SerializedGameData& sgd) const override;

    RoadSegment* GetRoute(const Direction dir) const { return routes[dir]; }
    void SetRoute(const Direction dir, RoadSegment* route) { routes[dir] = route; }
    const auto& getRoutes() const { return routes; }
    noRoadNode* GetNeighbour(Direction dir) const;

    void DestroyRoad(Direction dir);
    void UpgradeRoad(Direction dir) const;
    /// Vernichtet Alle Straße um diesen Knoten
    void DestroyAllRoads();

    unsigned char GetPlayer() const { return player; }

    /// Legt eine Ware am Objekt ab (an allen Straßenknoten (Gebäude, Baustellen und Flaggen) kann man Waren ablegen
    virtual void AddWare(std::unique_ptr<Ware> ware) = 0;

    /// Nur für Flagge, Gebäude können 0 zurückgeben, gibt Wegstrafpunkte für das Pathfinden für Waren, die in eine
    /// bestimmte Richtung noch transportiert werden müssen
    virtual unsigned GetPunishmentPoints(Direction) const { return 0; }
};

inline noRoadNode* noRoadNode::GetNeighbour(const Direction dir) const
{
    const RoadSegment* route = GetRoute(dir);
    if(!route)
        return nullptr;
    else if(route->GetF1() == this)
        return route->GetF2();
    else
        return route->GetF1();
}
