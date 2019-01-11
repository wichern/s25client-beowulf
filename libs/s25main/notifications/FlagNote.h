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

#ifndef FlagNote_h__
#define FlagNote_h__

#include "notifications/notifications.h"
#include "gameTypes/MapCoordinates.h"

struct FlagNote
{
    ENABLE_NOTIFICATION(FlagNote);

    enum Type
    {
        Constructed, // Construction succeeded
        ConstructionFailed, // Could not be constructed
        Destroyed, // Flag was destroyed
        DestructionFailed, // A destruction request failed
        Captured // Flag was captured (attached military building)
    };

    FlagNote(Type type, const MapPoint& pt, unsigned player) : type(type), pt(pt), player(player) {}

    const Type type;
    const MapPoint pt;
    const unsigned player;
};

#endif // FlagNote_h__
