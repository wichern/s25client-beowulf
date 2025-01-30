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
#include <pybind11/embed.h>

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

py::object AILoader::Create(unsigned idx)
{
    py::module module = py::module_::import(ais_[idx].dir.c_str());
    py::object python_class = module.attr(ais_[idx].name.c_str());
    return std::move(python_class());
}
