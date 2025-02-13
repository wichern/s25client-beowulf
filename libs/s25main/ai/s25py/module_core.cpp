// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_core.h"

#include "PyPlayer.h"
#include "gameTypes/AIInfo.h"
#include "ai/AIResource.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/BuildingType.h"
#include "gameTypes/MapCoordinates.h"
#include "gameTypes/GameSettingTypes.h"
#include "buildings/nobBaseWarehouse.h"
#include "s25util/System.h"

#include <boost/nowide/iostream.hpp>
namespace bnw = boost::nowide;
namespace py = pybind11;

namespace s25py {

void init_core(py::module_ &m)
{
    py::enum_<GameObjective>(m, "GameObjective")
      .value("NoObjective", GameObjective::None)
      .value("Conquer3_4", GameObjective::Conquer3_4)
      .value("TotalDomination", GameObjective::TotalDomination)
      .value("EconomyMode", GameObjective::EconomyMode)
      .value("Tournament1", GameObjective::Tournament1)
      .value("Tournament2", GameObjective::Tournament2)
      .value("Tournament3", GameObjective::Tournament3)
      .value("Tournament4", GameObjective::Tournament4)
      .value("Tournament5", GameObjective::Tournament5)
      .export_values();

    py::class_<s25py::PyPlayer, std::shared_ptr<s25py::PyPlayer>, s25py::PlayerTrampoline>(m, "Player")
      .def(py::init<>())
      .def("run_gf", &s25py::PyPlayer::RunGF)
      .def("on_chat_message", &s25py::PyPlayer::OnChatMessage)
      .def("get_headquater", &s25py::PyPlayer::GetHeadquater);

    py::class_<nobBaseWarehouse>(m, "BaseWarehouse")
      .def_property_readonly("type", &nobBaseWarehouse::GetBuildingType)
      .def_property_readonly("pos", &nobBaseWarehouse::GetPos)
      .def_property_readonly("flag_pos", &nobBaseWarehouse::GetFlagPos);

    py::class_<MapPoint>(m, "MapPoint")
      .def_readwrite("x", &MapPoint::x)
      .def_readwrite("y", &MapPoint::y)
      .def("__str__", [](const MapPoint& p) { return std::to_string(p.x) + ":" + std::to_string(p.y); });

    py::enum_<AI::Level>(m, "AILevel")
      .value("Easy", AI::Level::Easy)
      .value("Medium", AI::Level::Medium)
      .value("Hard", AI::Level::Hard)
      .export_values();

    py::enum_<BuildingQuality>(m, "BuildingQuality")
      .value("Nothing", BuildingQuality::Nothing)
      .value("Flag", BuildingQuality::Flag)
      .value("Mine", BuildingQuality::Mine)
      .value("Hut", BuildingQuality::Hut)
      .value("House", BuildingQuality::House)
      .value("Castle", BuildingQuality::Castle)
      .value("Harbor", BuildingQuality::Harbor)
      .export_values();

    py::enum_<BuildingType>(m, "BuildingType")
      .value("Headquarters", BuildingType::Headquarters)
      .value("Barracks", BuildingType::Barracks)
      .value("Guardhouse", BuildingType::Guardhouse)
      .value("Nothing2", BuildingType::Nothing2)
      .value("Watchtower", BuildingType::Watchtower)
      .value("Vineyard", BuildingType::Vineyard)
      .value("Winery", BuildingType::Winery)
      .value("Temple", BuildingType::Temple)
      .value("Nothing6", BuildingType::Nothing6)
      .value("Fortress", BuildingType::Fortress)
      .value("GraniteMine", BuildingType::GraniteMine)
      .value("CoalMine", BuildingType::CoalMine)
      .value("IronMine", BuildingType::IronMine)
      .value("GoldMine", BuildingType::GoldMine)
      .value("LookoutTower", BuildingType::LookoutTower)
      .value("Nothing7", BuildingType::Nothing7)
      .value("Catapult", BuildingType::Catapult)
      .value("Woodcutter", BuildingType::Woodcutter)
      .value("Fishery", BuildingType::Fishery)
      .value("Quarry", BuildingType::Quarry)
      .value("Forester", BuildingType::Forester)
      .value("Slaughterhouse", BuildingType::Slaughterhouse)
      .value("Hunter", BuildingType::Hunter)
      .value("Brewery", BuildingType::Brewery)
      .value("Armory", BuildingType::Armory)
      .value("Metalworks", BuildingType::Metalworks)
      .value("Ironsmelter", BuildingType::Ironsmelter)
      .value("Charburner", BuildingType::Charburner)
      .value("PigFarm", BuildingType::PigFarm)
      .value("Storehouse", BuildingType::Storehouse)
      .value("Nothing9", BuildingType::Nothing9)
      .value("Mill", BuildingType::Mill)
      .value("Bakery", BuildingType::Bakery)
      .value("Sawmill", BuildingType::Sawmill)
      .value("Mint", BuildingType::Mint)
      .value("Well", BuildingType::Well)
      .value("Shipyard", BuildingType::Shipyard)
      .value("Farm", BuildingType::Farm)
      .value("DonkeyBreeder", BuildingType::DonkeyBreeder)
      .value("HarborBuilding", BuildingType::HarborBuilding)
      .export_values();

    py::enum_<AIResource>(m, "ResourceType")
      .value("Gold", AIResource::Gold)
      .value("Ironore", AIResource::Ironore)
      .value("Coal", AIResource::Coal)
      .value("Granite", AIResource::Granite)
      .value("Fish", AIResource::Fish)
      .value("Wood", AIResource::Wood)
      .value("Stones", AIResource::Stones)
      .value("Plantspace", AIResource::Plantspace)
      .value("Borderland", AIResource::Borderland)
      .export_values();
}

}  // namespace s25py
