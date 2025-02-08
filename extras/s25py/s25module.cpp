// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "s25module.h"
#include "module_core.h"
#include "PyGame.h"
#include "PyPlayer.h"
#include "RTTR_Version.h"
#include "s25util/System.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

std::string version()
{
    return rttr::version::GetTitle() + " v" + rttr::version::GetVersion() + "-" + rttr::version::GetRevision() + "\n"
           + "Compiled with " + System::getCompilerName() + " for " + System::getOSName();
}

PYBIND11_MODULE(s25py, m)
{
    s25py::init_core(m);

    py::class_<s25py::PyGame>(m, "Game")
      .def(py::init<const std::string&, std::string, GameObjective, unsigned, unsigned>(),
        py::arg("map"),
           py::arg("replay") = "", 
           py::arg("objective") = GameObjective::TotalDomination,
           py::arg("random_seed") = 0,
           py::arg("networkframe_interval") = 20)
      .def("add_player", &s25py::PyGame::AddPlayer, py::keep_alive<1, 2>(), py::keep_alive<1, 4>())
      .def("add_player_aijh", &s25py::PyGame::AddPlayerAIJH, py::keep_alive<1, 2>())
      .def("run", &s25py::PyGame::Run,
           py::arg("max_gf") = std::numeric_limits<unsigned>::max())
      .def_property_readonly("current_gf", &s25py::PyGame::getCurrentGF)
      .def_property_readonly("statistic_buildings", &s25py::PyGame::getPlayerBuildings);
}
