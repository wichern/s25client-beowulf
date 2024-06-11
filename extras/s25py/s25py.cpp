// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

#include "s25py.h"

#include "gameTypes/AIInfo.h"
#include "RTTR_Version.h"
#include "s25util/System.h"

#include <boost/nowide/iostream.hpp>
namespace bnw = boost::nowide;

namespace s25py
{

std::string version()
{
    return rttr::version::GetTitle() + " v" + rttr::version::GetVersion() + "-"
                  + rttr::version::GetRevision() + "\n"
                  + "Compiled with " + System::getCompilerName() + " for " + System::getOSName();
}

void PlayerTrampoline::on_gameframe(bool gfisnwf)
{
    PYBIND11_OVERRIDE(
        void, /* Return type */
        PyPlayer,      /* Parent class */
        on_gameframe,          /* Name of function in C++ (must match Python name) */
        gfisnwf      /* Argument(s) */
    );
}

}  // namespace s25py

PYBIND11_MODULE(s25py, m) {
    m.doc() = "python plugin for s25 AI battles";

    m.def("version", &s25py::version);

    py::class_<s25py::PyGame>(m, "Game")
        .def(py::init<const std::string &>())
        .def_property("objective", &s25py::PyGame::getObjective, &s25py::PyGame::setObjective)
        .def_readwrite("save_replay", &s25py::PyGame::saveReplay_)
        .def("add_player", &s25py::PyGame::AddPlayer)
        .def("start", &s25py::PyGame::Start)
        .def("step", &s25py::PyGame::Step)
        .def("stop", &s25py::PyGame::Stop);

    py::class_<s25py::PyPlayer, s25py::PlayerTrampoline>(m, "Player")
        .def(py::init<const std::string &>())
        .def("on_gameframe", &s25py::PyPlayer::on_gameframe);

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

    py::enum_<AI::Level>(m, "AILevel")
        .value("Easy", AI::Level::Easy)
        .value("Medium", AI::Level::Medium)
        .value("Hard", AI::Level::Hard)
        .export_values();
}
