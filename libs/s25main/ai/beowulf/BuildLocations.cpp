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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Helper.h"

#include <utility> /* std::pair */
#include <algorithm> /* std::max */

namespace beowulf {

BuildLocations::BuildLocations(const MapBase& world)
{
    map_.Resize(world.GetSize());
}

BuildLocations::~BuildLocations()
{
    while (first_) {
        Node* n = first_;
        first_ = n->next;
        delete n;
    }

    while (freelist_) {
        Node* n = freelist_;
        freelist_ = n->next;
        delete n;
    }
}

void BuildLocations::Calculate(
        const World& world,
        const MapPoint& start)
{
    RTTR_Assert(start.isValid());

    while (first_)
        Remove(first_);

    FloodFill(map_, start,
    // condition
    [&world](const MapPoint& pos, Direction dir)
    {
        return world.IsRoadPossible(pos, dir);
    },
    // action
    [this, &world](const MapPoint& pos)
    {
        BuildingQuality bq = world.GetBQ(pos);
        if (bq > BQ_FLAG)
            Add(pos, bq);
    });
}

void BuildLocations::Update(
        const World& world,
        const MapPoint& pos,
        unsigned radius)
{
    // @todo: We need to check for every point whether it can still be connected
    //        to the destination road network.

    // there is no point in updating a radius less than 2:
    radius = std::max(radius, unsigned(2));

    map_.VisitPointsInRadius(pos, radius, [&](const MapPoint& pt)
    {
        BuildingQuality bq = world.GetBQ(pt);
        Node* node = map_[pt];
        if (node) {
            if (bq != node->bq) {
                Remove(node);
                if (bq > BQ_FLAG)
                    Add(pt, bq);
            }
        } else {
            if (bq > BQ_FLAG) {
                Add(pt, bq);
            }
        }
    }, true);
}

std::vector<MapPoint> BuildLocations::Get(
        BuildingQuality bq) const
{
    std::vector<MapPoint> ret;
    ret.reserve(size_);
    for (Node* n = first_; n != nullptr; n = n->next) {
        if (canUseBq(n->bq, bq)) {
            ret.push_back(n->pos);
        }
    }
    return ret;
}

BuildingQuality BuildLocations::Get(const MapPoint& pos) const
{
    Node* node = map_[pos];
    if (node)
        return node->bq;
    return BQ_NOTHING;
}

std::vector<MapPoint> BuildLocations::GetNearest(
        const MapPoint& pos,
        unsigned amount,
        BuildingQuality bq) const
{
    typedef std::pair<MapPoint, unsigned> point_t;
    struct Less {
        bool operator()(const point_t& l, const point_t& r) {
            return l.second < r.second;
        }
    };
    std::priority_queue<point_t, std::vector<point_t>, Less> queue;

    for (const MapPoint& pt : Get(bq)) {
        queue.push({ pt, map_.CalcDistance(pos, pt) });
    }

    std::vector<MapPoint> ret;
    for (unsigned i = 0; i < amount && !queue.empty(); ++i) {
        ret.push_back(queue.top().first);
        queue.pop();
    }

    return ret;
}

unsigned BuildLocations::GetSum() const
{
    return sum_;
}

unsigned BuildLocations::GetSize() const
{
    return size_;
}

void BuildLocations::Add(
        const MapPoint& pos,
        BuildingQuality bq)
{
    Node* node;
    if (freelist_) {
        node = freelist_;
        freelist_ = node->next;
        node->bq = bq;
        node->pos = pos;
        node->next = first_;
        node->prev = nullptr;
    } else {
        node = new Node{ bq, pos, first_, nullptr };
    }
    if (first_)
        first_->prev = node;
    first_ = node;
    map_[pos] = node;
    sum_ += static_cast<unsigned>(bq) - 1;
    size_++;
}

void BuildLocations::Remove(Node* node)
{
    sum_ -= static_cast<unsigned>(node->bq) - 1;
    map_[node->pos] = nullptr;

    if (first_ == node)
        first_ = node->next;

    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;
    node->next = freelist_;
    freelist_ = node;
    size_--;
}

} // namespace beowulf
