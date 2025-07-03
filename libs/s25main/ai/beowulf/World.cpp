// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/World.h"

#include "world/BQCalculator.h"
#include "world/GameWorldBase.h"

namespace beowulf {

World::World(GameWorldBase& gwb)
 : gwb_(gwb)
{

}

World::~World()
{

}

BuildingQuality World::GetBQ(const MapPoint& pt, const MapPoint& plannedPt, BuildingQuality plannedBq) const
{
    // We create a subclass of BQCalculator in order to override GetBM() so that it 
    // considers the planned building as well.
    class BQCalculatorWithPlannedBuilging : public BQCalculator
    {
    public:
        BQCalculatorWithPlannedBuilging(const GameWorldBase& gwb, const MapPoint& plannedPt, BuildingQuality plannedBq) :
            BQCalculator(gwb), plannedPt_(plannedPt), plannedBq_(plannedBq) {}
        virtual ~BQCalculatorWithPlannedBuilging() = default;

    private:
        BlockingManner GetBM(const MapPoint& pt) const override
        {
            BlockingManner bm = BlockingManner::None;

            if(plannedPt_ == pt)
            {
                if(BuildingQuality::Flag == plannedBq_)
                    return BlockingManner::Flag;
                return BlockingManner::Building;
            }
            
            if(BuildingQuality::Castle == plannedBq_)
            {
                if(world.CalcDistance(pt, plannedPt_) == 1)
                {
                    for(const auto dir : helpers::enumRange<Direction>())
                    {
                        if(plannedPt_ == world.GetNeighbour(pt, Direction(dir)))
                            return BlockingManner::Single;
                    }
                }
            }

            return world.GetNO(pt)->GetBM();
        }

        const MapPoint& plannedPt_;
        BuildingQuality plannedBq_;
    } calcBQ(gwb_, plannedPt, plannedBq);
    return calcBQ(pt, [&](auto pt) { return world.IsOnRoad(pt); });
}

} // namespace beowulf