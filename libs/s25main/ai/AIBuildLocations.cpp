// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/AIBuildLocations.h"
#include "ai/FloodFill.h"
#include "pathfinding/FreePathFinder.h"
#include "world/GameWorld.h"

AIBuildLocations::AIBuildLocations(const GameWorld& world) : world_(world)
{
    map_.Resize(world.GetSize());
}

AIBuildLocations::~AIBuildLocations() {}

void AIBuildLocations::Calculate(const MapPoint& start)
{
    RTTR_Assert(start.isValid());

    regionPt = start;

    // Clear all previous build locations.
    nodes_.clear();

    PathConditionRoad<GameWorldBase> roadPathChecker(world_, false);

    std::vector<Node> locations;
    FloodFill(map_, regionPt, roadPathChecker,
              // action
              [&](const MapPoint& pt) {
                  BuildingQuality bq = world_.GetBQ(pt);
                  if(bq > BuildingQuality::Flag)
                      locations.push_back({bq, pt});
              });

    for(const auto& loc : locations)
    {
        // Check if we can still connect that connection if we place a building.

        bool boat = false;
        if(world_.GetFreePathFinder().FindPathAlternatingConditions(
             start,                      // start
             loc.pos,                    // dest
             false,                      // random route
             200,                        // maxLength
             nullptr,                    // out: route
             nullptr,                    // out: length
             nullptr,                    // out: first direction
             IsPointOK_RoadPath,         // NodeOk Checker (@todo: replace with lamdba function that also check planned
                                         // building)
             IsPointOK_RoadPathEvenStep, // NodeOk Even checker  (@todo: replace with lamdba function that also check
                                         // planned building)
             nullptr,                    // IsNodeToDestOk
             (void*)&boat))
        {
            nodes_.push_back(loc);
        }
    }
}

void AIBuildLocations::Update(const MapPoint& pos, unsigned radius)
{
    // there is no point in updating a radius less than 2:
    radius = std::max(radius, unsigned(2));

    map_.VisitPointsInRadius(
      pos, radius,
      [&](const MapPoint& pt) {
          BuildingQuality bq = world_.GetBQ(pt, includeAnticipated_);
          Node* node = map_[pt];
          if(node)
          {
              if(bq != node->bq)
              {
                  Remove(node);
                  if(bq > BQ_FLAG)
                      Add(pt, bq);
              }
          } else
          {
              if(bq > BQ_FLAG)
              {
                  Add(pt, bq);
              }
          }
      },
      true);

    // Check all locations whether they can still be connected.
    for(Node* n = first_; n; n = n->next)
    {
        MapPoint flagPt = world_.GetNeighbour(n->pos, Direction::SOUTHEAST);
        std::vector<std::pair<MapPoint, BuildingQuality>> tmps = {{n->pos, n->bq}};
        if(!world_.CanConnectBuilding(flagPt, regionPt, includeAnticipated_, tmps))
        {
            Node* newPrev = n->prev;
            Remove(n);
            n = newPrev;
            if(!n)
                break;
        }
    }
}

std::vector<MapPoint> AIBuildLocations::Get() const
{
    std::vector<MapPoint> ret;
    ret.reserve(size_);
    for(Node* n = first_; n != nullptr; n = n->next)
        ret.push_back(n->pos);
    return ret;
}

std::vector<MapPoint> AIBuildLocations::Get(BuildingQuality bq) const
{
    std::vector<MapPoint> ret;
    ret.reserve(size_);
    for(Node* n = first_; n != nullptr; n = n->next)
    {
        if(canUseBq(n->bq, bq))
        {
            ret.push_back(n->pos);
        }
    }
    return ret;
}

BuildingQuality AIBuildLocations::Get(const MapPoint& pt) const
{
    Node* node = map_[pt];
    if(node)
        return node->bq;
    return BQ_NOTHING;
}

std::vector<MapPoint> AIBuildLocations::GetNearest(const MapPoint& pos, BuildingQuality bq, unsigned amount) const
{
    typedef std::pair<MapPoint, unsigned> point_t;
    struct Less
    {
        bool operator()(const point_t& l, const point_t& r) { return l.second < r.second; }
    };
    std::priority_queue<point_t, std::vector<point_t>, Less> queue;

    for(const MapPoint& pt : Get(bq))
    {
        queue.push({pt, map_.CalcDistance(pos, pt)});
    }

    std::vector<MapPoint> ret;
    for(unsigned i = 0; i < amount && !queue.empty(); ++i)
    {
        ret.push_back(queue.top().first);
        queue.pop();
    }

    return ret;
}

unsigned AIBuildLocations::GetSum() const
{
    return sum_;
}

unsigned AIBuildLocations::GetSize() const
{
    return size_;
}

void AIBuildLocations::Add(const MapPoint& pos, BuildingQuality bq)
{
    Node* node;
    if(freelist_)
    {
        node = freelist_;
        freelist_ = node->next;
        node->bq = bq;
        node->pos = pos;
        node->next = first_;
        node->prev = nullptr;
    } else
    {
        node = new Node{bq, pos, first_, nullptr};
    }
    if(first_)
        first_->prev = node;
    first_ = node;
    map_[pos] = node;
    sum_ += static_cast<unsigned>(bq) - 1;
    size_++;
}

void AIBuildLocations::Remove(Node* node)
{
    sum_ -= static_cast<unsigned>(node->bq) - 1;
    map_[node->pos] = nullptr;

    if(first_ == node)
        first_ = node->next;

    if(node->prev)
        node->prev->next = node->next;
    if(node->next)
        node->next->prev = node->prev;
    node->next = freelist_;
    freelist_ = node;
    size_--;
}
