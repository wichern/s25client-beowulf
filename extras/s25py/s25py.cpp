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

#include "RTTR_Version.h"
#include "gameTypes/AIInfo.h"
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
    PYBIND11_OVERRIDE(void,       /* Return type */
                      PyPlayer,   /* Parent class */
                      RunGF,      /* Name of function in C++ (must match Python name) */
                      gf, gfisnwf /* Argument(s) */
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
      .def("add_player", &s25py::PyGame::AddPlayer)
      .def("next_gameframe", &s25py::PyGame::Step)
      .def_property_readonly("current_gf", &s25py::PyGame::getCurrentGF)
      .def_property_readonly("statistic_buildings", &s25py::PyGame::getPlayerBuildings);

    // We have to use a trampoline class for PyPlayer in order for python to create subclasses.
    // We have to use shared_ptr encapsulation in order to allow passing ownership to PyGame.
    py::class_<s25py::PyPlayer, s25py::PlayerTrampoline, std::shared_ptr<s25py::PyPlayer>>(m, "Player")
      .def(py::init<const std::string&>())
      .def("next_gameframe", &s25py::PyPlayer::RunGF);

    py::class_<s25py::PyPlayerAIJH, std::shared_ptr<s25py::PyPlayerAIJH>, s25py::PyPlayer>(m, "PlayerAIJH")
      .def(py::init<const std::string&>())
      .def("next_gameframe", &s25py::PyPlayer::RunGF);

    py::enum_<AI::Level>(m, "AILevel")
      .value("Easy", AI::Level::Easy)
      .value("Medium", AI::Level::Medium)
      .value("Hard", AI::Level::Hard)
      .export_values();
}
