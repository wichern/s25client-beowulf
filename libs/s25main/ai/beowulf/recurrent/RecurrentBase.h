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
#ifndef BEOWULF_RECURRENT_RECURRENTBASE_H_INCLUDED
#define BEOWULF_RECURRENT_RECURRENTBASE_H_INCLUDED

#include "notifications/Subscribtion.h"

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
    std::vector<Subscribtion> events_;

    unsigned interval_;
    unsigned intervalCounter_;
};

} // namespace beowulf

#endif //! BEOWULF_RECURRENT_RECURRENTBASE_H_INCLUDED
