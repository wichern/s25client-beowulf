// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

#include "s25py.h"

#include "HeadlessGame.h"
#include "RttrConfig.h"

#include <boost/nowide/iostream.hpp>

namespace bnw = boost::nowide;
namespace py = pybind11;

namespace s25py
{

Game::Game(const std::string& map)
: map_(map)
{
}

Game::~Game()
{
}

void Game::ActivateReplay(bool activate)
{
    saveReplay_ = activate;
}

void Game::Start()
{
}

void Game::Step()
{
}

void Game::Stop()
{
}

Player::Player(const std::string& name)
{
    (void)name;
}

Player::~Player()
{

}

PlayerAIJH::PlayerAIJH(const std::string& name)
: Player(name)
{

}

PlayerAIJH::~PlayerAIJH()
{

}

}  // namespace s25py

PYBIND11_MODULE(s25py, m) {
    m.doc() = "python plugin for s25 AI battles";

    //-------------------------------------------------------------------------
    // General Types

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

    //-------------------------------------------------------------------------
    // s25py Classes

    py::class_<s25py::Game>(m, "Game")
        .def(py::init<const std::string &>())
        .def_property("objective", &s25py::Game::getObjective, &s25py::Game::setObjective)
        .def_readwrite("save_replay", &s25py::Game::saveReplay_)
        .def("Start", &s25py::Game::Start)
        .def("Step", &s25py::Game::Step)
        .def("Stop", &s25py::Game::Stop);

    py::class_<s25py::Player>(m, "Player")
        .def(py::init<const std::string &>());
}
