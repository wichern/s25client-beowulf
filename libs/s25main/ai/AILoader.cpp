// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/AILoader.h"
#include "ListDir.h"
#include "RttrConfig.h"
#include "ai/s25py/pyPlayer.h"
#include "ai/s25py/s25py.h"
#include "files.h"
#include <iostream>

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/embed.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

PYBIND11_EMBEDDED_MODULE(s25py, m)
{
    py::class_<s25py::PyPlayer, std::shared_ptr<s25py::PyPlayer>, s25py::PlayerTrampoline>(m, "Player")
      .def(py::init<>())
      .def("next_gameframe", &s25py::PyPlayer::RunGF)
      .def("get_headquaters", &s25py::PyPlayer::GetHeadquaters);
}

AILoader::AILoader() : rootDir_(RTTRCONFIG.ExpandPath(s25::folders::ai))
{
    py::initialize_interpreter();
}

AILoader::~AILoader()
{
    py::finalize_interpreter();
}

void AILoader::Load()
{
    // Add AI asset dir to python sys path.
    py::module sys = py::module_::import("sys");
    py::list sys_path = sys.attr("path");
    sys_path.append(rootDir_.string());

    // List all subdirectories containing an __init__.py
    for(const auto& entry : bfs::directory_iterator(rootDir_))
    {
        try {
            if(bfs::is_directory(entry) && bfs::is_regular_file(entry.path() / "__init__.py"))
            {
                // @todo: Add every class that is a subclass of PyPlayer. The subclass name shall be the AI name
                try
                {
                    py::object aiPlayer = py::module_::import(entry.path().stem().string().c_str()).attr("AwesomeAI")();
                    ais_.push_back({entry.path().stem().string(), "AwesomeAI"});
                } catch(const py::error_already_set& e)
                {
                    std::cerr << "Failed to import module '" << entry.path().stem().string() << "': " << e.what()
                              << std::endl;
                }
            }
        } catch (const bfs::filesystem_error& ex) {
            std::cerr << "Filesystem error: " << ex.what() << std::endl;
        }
    }
}

std::unique_ptr<AIPlayer> AILoader::Create(unsigned idx)
{
    py::object python_class = py::module_::import(ais_[idx].dir.c_str()).attr(ais_[idx].name);
    py::object python_instance = python_class();
    return std::move(python_instance);
}
