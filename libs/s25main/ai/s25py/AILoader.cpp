// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/s25py/AILoader.h"
#include "ai/s25py/PyPlayer.h"
#include "commonDefines.h"
#include "ListDir.h"
#include "RttrConfig.h"
#include "files.h"
#include "s25util/error.h"
#include "s25util/Log.h"
#include <boost/filesystem.hpp>

namespace py = pybind11;

bool is_subclass(const py::handle& obj, const py::handle& super);
bool is_subclass(const py::handle& obj, const py::handle& super) {
    if (py::hasattr(obj, "__bases__")) {
        py::tuple bases = obj.attr("__bases__");
        for (const auto& base : bases) {
            if (super.equal(base))
                return true;
        }
    }
    return false;
}

void AILoader::Load()
{
    bfs::path rootDir = RTTRCONFIG.ExpandPath(s25::folders::ai);

    // Add AI asset dir to python sys path.
    py::module sys = py::module_::import("sys");
    py::list sys_path = sys.attr("path");
    sys_path.append(rootDir.string());

    py::object base_class = py::module_::import("s25py").attr("Player");

    // List all subdirectories containing an __init__.py
    for(const auto& entry : bfs::directory_iterator(rootDir))
    {
        try {
            if(!bfs::is_directory(entry) && bfs::is_regular_file(entry.path() / "__init__.py"))
                continue;

            // Loop over all classes in the module and extract those that are subclasses of Player
            try
            {
                const std::string subfolder = entry.path().filename().string();
                py::module_ module = py::module_::import(subfolder.c_str());

                for(auto item : module.attr("__dict__").cast<py::dict>()) {
                    if (is_subclass(item.second, base_class)) {
                        LOG.write("Import AI: %s\n", LogTarget::Stdout) % item.first.cast<std::string>().c_str();
                        ais_.push_back({subfolder, item.first.cast<std::string>()});
                    }
                }
            } catch(const py::error_already_set& ex)
            {
                s25util::error(std::string("Python error: ") + ex.what());
            }
        } catch(const bfs::filesystem_error& ex)
        {
            s25util::error(std::string("Filesystem error: ") + ex.what());
        }
    }
}

py::object AILoader::Create(unsigned idx)
{
    py::module module = py::module_::import(ais_[idx].dir.c_str());
    py::object python_class = module.attr(ais_[idx].name.c_str());
    return std::move(python_class());
}

unsigned AILoader::GetIdx(const std::string& name) const
{
    for(const auto& ai : ais_)
    {
        if(ai.name == name)
            return &ai - &ais_[0];
    }
    return std::numeric_limits<unsigned>::max();
}
