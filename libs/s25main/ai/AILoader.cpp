// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/AILoader.h"
#include "ListDir.h"
#include "files.h"
#include "RttrConfig.h"
#include <iostream>

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/embed.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

AILoader::AILoader()
{
    py::scoped_interpreter guard{};
}

AILoader::~AILoader()
{
}

void AILoader::Load()
{
    const bfs::path root = RTTRCONFIG.ExpandPath(s25::folders::ai);
    
    // List all subdirectories containing an __init__.py
    for (const auto& entry : bfs::directory_iterator(root)) {
        try {
            if(bfs::is_regular_file(entry) && entry.path().filename() == "__init__.py")
                Add(entry.path().parent_path());
        } catch (const bfs::filesystem_error& ex) {
            std::cerr << "Filesystem error: " << ex.what() << std::endl;
        }
    }
}

void AILoader::Add(const bfs::path& dir)
{
    std::cout << "AI: " << dir.stem().string() << std::endl;

    py::module sys = py::module::import("sys");
    py::list sys_path = sys.attr("path");
    sys_path.append(dir.string());

    try {
        std::cout << "Importing Python module: " << dir << std::endl;
        py::module::import(dir.c_str());
    } catch (const py::error_already_set& e) {
        std::cerr << "Failed to import module '" << dir << "': " << e.what() << std::endl;
    }

    //py::object aiPlayer = py::module_::import("decimal").attr("Decimal");
}
