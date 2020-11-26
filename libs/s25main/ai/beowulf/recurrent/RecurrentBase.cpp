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

#include "ai/beowulf/recurrent/RecurrentBase.h"
#include "ai/beowulf/Beowulf.h"

#include "notifications/NotificationManager.h"
#include "notifications/BuildingNote.h"
#include "notifications/RoadNote.h"
#include "notifications/ToolNote.h"
#include "notifications/NodeNote.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>

namespace beowulf {

RecurrentBase::RecurrentBase(Beowulf* beowulf, unsigned interval, unsigned intervalCounter)
    : beowulf_(beowulf),
      interval_(interval)
{
    if (intervalCounter == std::numeric_limits<unsigned>::max())
        intervalCounter_ = interval_ / 4;
    else
        intervalCounter_ = intervalCounter;

    NotificationManager& notifications = beowulf_->GetAII().gwb.GetNotifications();

    events_.push_back(notifications.subscribe<BuildingNote>([this](const BuildingNote& note)
    {
        if (note.player == beowulf_->GetPlayerId()) OnBuildingNote(note);
    }));

    events_.push_back(notifications.subscribe<RoadNote>([this](const RoadNote& note)
    {
        if (note.player == beowulf_->GetPlayerId()) OnRoadNote(note);
    }));

    events_.push_back(notifications.subscribe<ToolNote>([this](const ToolNote& note)
    {
        if (note.player == beowulf_->GetPlayerId()) OnToolNote(note);
    }));

    events_.push_back(notifications.subscribe<NodeNote>([this](const NodeNote& note)
    {
        if (note.type == NodeNote::BQ && beowulf_->GetAII().gwb.IsPlayerTerritory(note.pos)) OnNodeNote(note);
    }));
}

void RecurrentBase::RunGf()
{
    if (!enabled_)
        return;

    if (0 == intervalCounter_) {
        OnRun();
        intervalCounter_ = interval_;
    } else {
        intervalCounter_--;
    }
}

void RecurrentBase::OnToolNote(const ToolNote& note)
{
    (void)note;
}

void RecurrentBase::OnBuildingNote(const BuildingNote& note)
{
    (void)note;
}

void RecurrentBase::OnRoadNote(const RoadNote& note)
{
    (void)note;
}

void RecurrentBase::OnNodeNote(const NodeNote& note)
{
    (void)note;
}

} // namespace beowulf
