// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "s25module.h"
#include "s25py.h"
#include "pyGame.h"
#include "pyPlayer.h"
#include "RTTR_Version.h"
#include "s25util/System.h"

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

std::string version()
{
    return rttr::version::GetTitle() + " v" + rttr::version::GetVersion() + "-" + rttr::version::GetRevision() + "\n"
           + "Compiled with " + System::getCompilerName() + " for " + System::getOSName();
}

PYBIND11_MODULE(s25py, m)
{
    s25py::init_core(m);

    py::class_<s25py::PyGame>(m, "Game")
      .def(py::init<const std::string&, std::string, GameObjective, unsigned, unsigned, unsigned>(), py::arg("map"),
           py::arg("replay") = "", py::arg("objective") = GameObjective::TotalDomination,
           py::arg("max_gameframe") = std::numeric_limits<unsigned>::max(), py::arg("random_seed") = 0,
           py::arg("networkframe_interval") = 20)
      .def("add_player", &s25py::PyGame::AddPlayer, py::keep_alive<1, 2>())
      .def("next_gameframe", &s25py::PyGame::Step)
      .def_property_readonly("current_gf", &s25py::PyGame::getCurrentGF)
      .def_property_readonly("statistic_buildings", &s25py::PyGame::getPlayerBuildings);

    // py::class_<s25py::PyPlayerAIJH, std::shared_ptr<s25py::PyPlayerAIJH>, s25py::PyPlayer>(m, "PlayerAIJH")
    //   .def(py::init<>())
    //   .def("next_gameframe", &s25py::PyPlayer::RunGF);
}
