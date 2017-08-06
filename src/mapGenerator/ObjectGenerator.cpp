// Copyright (c) 2017 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "defines.h" // IWYU pragma: keep
#include "mapGenerator/ObjectGenerator.h"
#include "mapGenerator/RandomConfig.h"
#include "libsiedler2/src/enumTypes.h"

bool ObjectGenerator::IsHarborAllowed(TerrainType terrain)
{
    switch(terrain)
    {
        case TT_STEPPE:
        case TT_SAVANNAH:
        case TT_MEADOW1:
        case TT_MEADOW2:
        case TT_MEADOW3:
        case TT_MEADOW_FLOWERS:
        case TT_MOUNTAINMEADOW: return true;
        default: return false;
    }
}

void ObjectGenerator::CreateTexture(Map& map, int index, TerrainType terrain, bool harbor)
{
    uint8_t textureId = harbor && IsHarborAllowed(terrain) ? TerrainData::GetTextureIdentifier(terrain) | libsiedler2::HARBOR_MASK :
                                                             TerrainData::GetTextureIdentifier(terrain);

    map.textureRsu[index] = textureId;
    map.textureLsd[index] = textureId;
}

bool ObjectGenerator::IsTexture(const Map& map, int index, TerrainType terrain)
{
    return map.textureRsu[index] == TerrainData::GetTextureIdentifier(terrain)
           || map.textureLsd[index] == TerrainData::GetTextureIdentifier(terrain);
}

void ObjectGenerator::CreateEmpty(Map& map, int index)
{
    map.objectType[index] = libsiedler2::OT_Empty;
    map.objectInfo[index] = libsiedler2::OI_Empty;
}

void ObjectGenerator::CreateHeadquarter(Map& map, int index, unsigned i)
{
    map.objectType[index] = i;
    map.objectInfo[index] = libsiedler2::OI_HeadquarterMask;
}

bool ObjectGenerator::IsEmpty(const Map& map, int index)
{
    return (map.objectType[index] == libsiedler2::OT_Empty && map.objectInfo[index] == libsiedler2::OI_Empty);
}

uint8_t ObjectGenerator::CreateDuck(int likelihood)
{
    return config.Rand(100) < likelihood ? libsiedler2::A_Duck : libsiedler2::A_None;
}

uint8_t ObjectGenerator::CreateSheep(int likelihood)
{
    return config.Rand(100) < likelihood ? libsiedler2::A_Sheep : libsiedler2::A_None;
}

uint8_t ObjectGenerator::CreateRandomForestAnimal(int likelihood)
{
    if(config.Rand(100) >= likelihood)
    {
        return libsiedler2::A_None;
    }

    switch(config.Rand(5))
    {
        case 0: return libsiedler2::A_Rabbit;
        case 1: return libsiedler2::A_Fox;
        case 2: return libsiedler2::A_Stag;
        case 3: return libsiedler2::A_Deer;
        default: return libsiedler2::A_Deer2;
    }
}

uint8_t ObjectGenerator::CreateRandomAnimal(int likelihood)
{
    if(config.Rand(100) >= likelihood)
    {
        return libsiedler2::A_None;
    }

    switch(config.Rand(7))
    {
        case 0: return libsiedler2::A_Rabbit;
        case 1: return libsiedler2::A_Fox;
        case 2: return libsiedler2::A_Stag;
        case 3: return libsiedler2::A_Deer;
        case 4: return libsiedler2::A_Sheep;
        case 5: return libsiedler2::A_Donkey;
        default: return libsiedler2::A_Deer2;
    }
}

uint8_t ObjectGenerator::CreateRandomResource(unsigned ratioGold, unsigned ratioIron, unsigned ratioCoal, unsigned ratioGranite)
{
    unsigned rnd = (unsigned)config.Rand(ratioGold + ratioIron + ratioCoal + ratioGranite);

    if(rnd <= ratioGold)
        return libsiedler2::R_Gold + config.Rand(8);
    else if(rnd <= ratioGold + ratioIron)
        return libsiedler2::R_Iron + config.Rand(8);
    else if(rnd <= ratioGold + ratioIron + ratioCoal)
        return libsiedler2::R_Coal + config.Rand(8);
    else
        return libsiedler2::R_Granite + config.Rand(8);
}

bool ObjectGenerator::IsTree(const Map& map, int index)
{
    return map.objectInfo[index] == libsiedler2::OI_TreeOrPalm || map.objectInfo[index] == libsiedler2::OI_Palm;
}

void ObjectGenerator::CreateRandomTree(Map& map, int index)
{
    switch(config.Rand(3))
    {
        case 0:
            map.objectType[index] = libsiedler2::OT_Tree1_Begin + config.Rand(libsiedler2::OT_Tree1_End - libsiedler2::OT_Tree1_Begin + 1);
            break;
        case 1:
            map.objectType[index] = libsiedler2::OT_Tree2_Begin + config.Rand(libsiedler2::OT_Tree2_End - libsiedler2::OT_Tree2_Begin + 1);
            break;
        case 2:
            map.objectType[index] =
              libsiedler2::OT_TreeOrPalm_Begin + config.Rand(libsiedler2::OT_TreeOrPalm_End - libsiedler2::OT_TreeOrPalm_Begin + 1);
            break;
    }
    map.objectInfo[index] = libsiedler2::OI_TreeOrPalm;
}

void ObjectGenerator::CreateRandomPalm(Map& map, int index)
{
    if(config.Rand(2) == 0)
    {
        map.objectType[index] =
          libsiedler2::OT_TreeOrPalm_Begin + config.Rand(libsiedler2::OT_TreeOrPalm_End - libsiedler2::OT_TreeOrPalm_Begin + 1);
        map.objectInfo[index] = libsiedler2::OI_Palm;
    } else
    {
        map.objectType[index] = libsiedler2::OT_Palm_Begin + config.Rand(0, libsiedler2::OT_Palm_End - libsiedler2::OT_Palm_Begin + 1);
        map.objectInfo[index] = libsiedler2::OI_TreeOrPalm;
    }
}

void ObjectGenerator::CreateRandomMixedTree(Map& map, int index)
{
    if(config.Rand(2) == 0)
    {
        CreateRandomTree(map, index);
    } else
    {
        CreateRandomPalm(map, index);
    }
}

void ObjectGenerator::CreateRandomStone(Map& map, int index)
{
    map.objectType[index] = libsiedler2::OT_Stone_Begin + config.Rand(libsiedler2::OT_Stone_End - libsiedler2::OT_Stone_Begin + 1);
    map.objectInfo[index] = config.Rand(2) == 0 ? libsiedler2::OI_Stone1 : libsiedler2::OI_Stone2;
}
