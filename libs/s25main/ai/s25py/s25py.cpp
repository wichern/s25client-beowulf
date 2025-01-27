// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

#include "pyPlayerAIJH.h"
#include "s25py.h"

#include "Point.h"
#include "RTTR_Version.h"
#include "gameTypes/AIInfo.h"
#include "gameTypes/BuildingQuality.h"
#include "gameTypes/BuildingType.h"
#include "s25util/System.h"

#include <boost/nowide/iostream.hpp>
namespace bnw = boost::nowide;

namespace s25py {

std::string version()
{
    return rttr::version::GetTitle() + " v" + rttr::version::GetVersion() + "-" + rttr::version::GetRevision() + "\n"
           + "Compiled with " + System::getCompilerName() + " for " + System::getOSName();
}

void PlayerTrampoline::RunGF(unsigned gf, bool gfisnwf)
{
    PYBIND11_OVERRIDE_NAME(void,             /* Return type */
                           PyPlayer,         /* Parent class */
                           "next_gameframe", /* Name of method in python */
                           RunGF,            /* Name of function in C++ */
                           gf, gfisnwf       /* Argument(s) */
    );
}

} // namespace s25py

PYBIND11_MODULE(s25py, m)
{
    m.doc() = "python plugin for s25 AI battles";

    m.def("version", &s25py::version);

    py::enum_<GameObjective>(m, "GameObjective")
      .value("None", GameObjective::None)
      .value("Conquer3_4", GameObjective::Conquer3_4)
      .value("TotalDomination", GameObjective::TotalDomination)
      .value("EconomyMode", GameObjective::EconomyMode)
      .value("Tournament1", GameObjective::Tournament1)
      .value("Tournament2", GameObjective::Tournament2)
      .value("Tournament3", GameObjective::Tournament3)
      .value("Tournament4", GameObjective::Tournament4)
      .value("Tournament5", GameObjective::Tournament5)
      .export_values();

    py::class_<s25py::PyGame>(m, "Game")
      .def(py::init<const std::string&, std::string, GameObjective, unsigned, unsigned, unsigned>(), py::arg("map"),
           py::arg("replay") = "", py::arg("objective") = GameObjective::TotalDomination,
           py::arg("max_gameframe") = std::numeric_limits<unsigned>::max(), py::arg("random_seed") = 0,
           py::arg("networkframe_interval") = 20)
      .def("add_player", &s25py::PyGame::AddPlayer, py::keep_alive<1, 2>())
      .def("next_gameframe", &s25py::PyGame::Step)
      .def_property_readonly("current_gf", &s25py::PyGame::getCurrentGF)
      .def_property_readonly("statistic_buildings", &s25py::PyGame::getPlayerBuildings);

    // We have to use a trampoline class for PyPlayer in order for python to create subclasses.
    // We have to use shared_ptr encapsulation in order to allow passing ownership to PyGame.
    py::class_<s25py::PyPlayer, std::shared_ptr<s25py::PyPlayer>, s25py::PlayerTrampoline>(m, "Player")
      .def(py::init<>())
      .def("next_gameframe", &s25py::PyPlayer::RunGF)
      .def("get_headquaters", &s25py::PyPlayer::GetHeadquaters);

    py::class_<s25py::PyBuilding>(m, "Building")
      .def_readwrite("type", &s25py::PyBuilding::type)
      .def_readwrite("pos", &s25py::PyBuilding::pos)
      .def_readwrite("flag_pos", &s25py::PyBuilding::flag_pos);

    py::class_<MapPoint>(m, "MapPoint")
      .def_readwrite("x", &MapPoint::x)
      .def_readwrite("y", &MapPoint::y)
      .def("__str__", [](const MapPoint& p) { return std::to_string(p.x) + ":" + std::to_string(p.y); });

    py::class_<s25py::PyPlayerAIJH, std::shared_ptr<s25py::PyPlayerAIJH>, s25py::PyPlayer>(m, "PlayerAIJH")
      .def(py::init<>())
      .def("next_gameframe", &s25py::PyPlayer::RunGF);

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
