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
#ifndef BEOWULF_DEBUG_H_INCLUDED
#define BEOWULF_DEBUG_H_INCLUDED

#include "ai/AIInterface.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/Direction.h"

#include <string>
#include <vector>

#include <boost/nowide/fstream.hpp>

namespace beowulf {

class BuildLocations;
class Buildings;

std::string to_string(unsigned val);
std::string to_string(const MapPoint& pt);
std::string to_string(const std::vector<unsigned>& intvec);
std::string to_string(const unsigned* intvec, unsigned len);
std::string to_string(Direction dir);
std::string to_string(const std::vector<Direction>& route);

void CreateSvg(const AIInterface& aii,
               const BuildLocations& bl,
               const std::string& path);

void CreateSvg(const AIInterface& aii,
               const Buildings& buildings,
               const std::string& path);

} // namespace beowulf

#endif //! BEOWULF_DEBUG_H_INCLUDED
