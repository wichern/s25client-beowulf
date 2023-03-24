// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "notifications/Subscription.h"

#include <vector>
#include <limits>

class BuildingNote;
class RoadNote;
class ToolNote;
class NodeNote;

namespace beowulf {

class Beowulf;

class RecurrentBase
{
public:
    /*
     * Recurrents will not be called on every gf.
     * But even the remaining calls on RunGF will be skipped for the 'interval' count.
     */
    RecurrentBase(Beowulf* beowulf, unsigned interval = 0, unsigned intervalCounter = std::numeric_limits<unsigned>::max());
    virtual ~RecurrentBase() {}

    // Called by Beowulf only after network synchronization frames occured.
    void RunGf();

    void Enable() { enabled_ = true; }
    void Disable() { enabled_ = false; }

protected:
    // Implement me.
    virtual void OnRun() = 0;

    virtual void OnToolNote(const ToolNote& note);
    virtual void OnBuildingNote(const BuildingNote& note);
    virtual void OnRoadNote(const RoadNote& note);
    virtual void OnNodeNote(const NodeNote& note);

    Beowulf* beowulf_;

    // Recurrents can be disabled completely (for unit testing).
    bool enabled_ = true;

private:
    std::vector<Subscription> events_;

    unsigned interval_;
    unsigned intervalCounter_;
};

} // namespace beowulf
