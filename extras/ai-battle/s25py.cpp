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
    bnw::cout << "Game(" << map << ")" << std::endl;
}

Game::~Game()
{
    bnw::cout << "~Game()" << std::endl;
}

void Game::AddPlayer(Player* player)
{
    if (!player)
        return;

    PlayerInfo p;
    p.ps = PlayerState::Occupied;
    p.aiInfo = {AI::Type::Default, AI::Level::Hard};
    p.name = player->name_;
    p.nation = Nation::Romans;
    p.team = Team::None;

    player->id_ = ais_.size();
    player->game_ = this;

    ais_.push_back(p);
    players_.push_back(player);
    bnw::cout << "Added Player " << player->name_ << std::endl;
}

void Game::ActivateReplay(bool activate)
{
    saveReplay_ = activate;
}

void Game::Start()
{
    bnw::cout << "Start()" << std::endl;
}

void Game::Step()
{
    bnw::cout << "Step()" << std::endl;
    for (Player* p : players_)
        p->on_gameframe(false);
}

void Game::Stop()
{
    bnw::cout << "Stop()" << std::endl;
}

Player::Player(const std::string& name)
: name_(name)
{
    bnw::cout << "Player()" << std::endl;
}

Player::~Player()
{
    bnw::cout << "~Player()" << std::endl;
}

void Player::on_gameframe(bool /*gfisnwf*/)
{
    bnw::cout << "on_gameframe base class" << std::endl;
}

}  // namespace s25py

PYBIND11_MODULE(s25py, m) {
    m.doc() = "python plugin for s25 AI battles";

    py::class_<s25py::Game>(m, "Game")
        .def(py::init<const std::string &>())
        .def_property("objective", &s25py::Game::getObjective, &s25py::Game::setObjective)
        .def_readwrite("save_replay", &s25py::Game::saveReplay_)
        .def("add_player", &s25py::Game::AddPlayer)
        .def("start", &s25py::Game::Start)
        .def("step", &s25py::Game::Step)
        .def("stop", &s25py::Game::Stop);

    py::class_<s25py::Player, s25py::PyPlayer>(m, "Player")
        .def(py::init<const std::string &>())
        .def("on_gameframe", &s25py::Player::on_gameframe);

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
