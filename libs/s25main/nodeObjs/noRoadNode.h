// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "RoadSegment.h"
#include "helpers/EnumArray.h"
#include "noCoordBase.h"
#include "gameTypes/Direction.h"
#include "gameTypes/RoadPathDirection.h"
#include "pathfinding/DStarTypes.h"
#include <map>

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

    noRoadNode(NodalObjectType nop, MapPoint pos, unsigned char player);
    noRoadNode(SerializedGameData& sgd, unsigned obj_id);
    noRoadNode(const noRoadNode&) = delete;
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

    mutable dstar::GoalContainer<dstar::NodeState> dstar;
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

// NeighbourRange — yields (Direction, const noRoadNode*)
template <typename NodeT>
struct NeighbourRange {
    static_assert(std::is_same_v<std::remove_const_t<NodeT>, noRoadNode>,
                  "NeighbourRange must be instantiated with (const) noRoadNode*");

    using NodePtr = NodeT*;
    using RoadArray = helpers::EnumArray<RoadSegment*, Direction>;

    NodePtr node;
    RoadArray routes;  // stored by value (cheap, avoids dangling reference)

    explicit NeighbourRange(NodePtr n)
        : node(n), routes(n->getRoutes()) {}

    struct Neighbour {
        Direction dir;
        NodePtr Neighbour;
    };

    struct iterator {
        NodePtr node;
        const RoadArray* routes;
        unsigned dirIndex;
        static constexpr unsigned dirCount = helpers::NumEnumValues_v<Direction>;
        Neighbour current{};

        iterator(NodePtr n, const RoadArray* r, unsigned idx)
            : node(n), routes(r), dirIndex(idx)
        {
            if (dirIndex < dirCount)
                advance();
        }

        void advance() {
            while (dirIndex < dirCount) {
                Direction dir = static_cast<Direction>(dirIndex++);
                const auto* route = (*routes)[dir];
                if (!route)
                    continue;

                NodePtr neighbour = route->GetF1();
                if (neighbour == node)
                    neighbour = route->GetF2();

                current = {dir, neighbour};
                return;
            }
            dirIndex = dirCount;  // mark as end
            current = {static_cast<Direction>(0), nullptr};
        }

        Neighbour operator*() const noexcept { return current; }
        iterator& operator++() { advance(); return *this; }
        bool operator!=(const iterator& other) const noexcept { return dirIndex != other.dirIndex; }
    };

    iterator begin() const noexcept { return iterator{node, &routes, 0u}; }
    iterator end() const noexcept { return iterator{node, &routes, iterator::dirCount}; }
};
