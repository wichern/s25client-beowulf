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

#include "ai/beowulf/BuildingQualityCalculator.h"
#include "ai/beowulf/Types.h"
#include "ai/AIInterface.h"
#include "pathfinding/PathConditionRoad.h"
#include "nodeObjs/noFlag.h"

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

namespace beowulf {

BuildingQualityCalculator::BuildingQualityCalculator(const AIInterface& aii)
    : BQCalculator(aii.gwb), aii_(aii)//, edgeChecker_(aii.gwb.GetSize())
{

}

BuildingQualityCalculator::BuildingQualityCalculator(
        const BuildingQualityCalculator& other)
    : BQCalculator(other.aii_.gwb),
      aii_(other.aii_),
      bmProvider_(other.bmProvider_)
{

}

void BuildingQualityCalculator::AddBlockingReason(IBlockingReason* reason)
{
    bmProvider_.push_back(reason);
}

void BuildingQualityCalculator::RemoveBlockingReason(IBlockingReason* reason)
{
    bmProvider_.erase(
                std::remove(
                    bmProvider_.begin(),
                    bmProvider_.end(),
                    reason),
                bmProvider_.end());
}

void BuildingQualityCalculator::AddRoadProvider(IRoadProvider* provider)
{
    roadProvider_.push_back(provider);
}

void BuildingQualityCalculator::RemoveRoadProvider(IRoadProvider* provider)
{
    roadProvider_.erase(
                std::remove(
                    roadProvider_.begin(),
                    roadProvider_.end(),
                    provider),
                roadProvider_.end());
}

BuildingQuality BuildingQualityCalculator::GetBQ(const MapPoint& pt) const
{
    BuildingQuality bq = (*this)(pt, boost::bind(&BuildingQualityCalculator::IsOnRoad, this, _1));
    return world.AdjustBQ(pt, aii_.GetPlayerId(), bq);
}

BlockingManner BuildingQualityCalculator::GetBM(const MapPoint& pt) const
{
    for (const IBlockingReason* reason : bmProvider_) {
        BlockingManner bm = reason->GetBM(pt);
        if (bm != BlockingManner::None)
            return bm;
    }
    return world.GetNO(pt)->GetBM();
}

bool BuildingQualityCalculator::IsOnRoad(const MapPoint& pt) const
{
    if (aii_.gwb.IsOnRoad(pt)) // @todo: should we remove this check in order to ignore roads that might already be marked for destruction?
        return true;

    for (const IRoadProvider* provider : roadProvider_)
        if (provider->IsOnRoad(pt))
            return true;

    return false;
}

} // namespace beowulf
