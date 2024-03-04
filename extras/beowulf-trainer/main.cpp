// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "files.h"
#include "random/Random.h"
#include "RTTR_Version.h"
#include "RttrConfig.h"
#include "s25util/System.h"
#include "QuickStartGame.h"
#include "TrainingGame.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/program_options.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    // @todo: create replay
    // @todo: which seed to use?
    // @todo: support different abort conditions (gf-limit, player X won)
    // @todo: support loading maps from different subdirectories

    bnw::args _(argc, argv);

    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help,h", "Show help")
        ("map,m", po::value<std::string>(),"Map to load")
        ("ai", po::value<std::vector<std::string>>(),"AI player(s) to add")
        ("version", "Show version information and exit")
        ;
    // clang-format on
    po::positional_options_description positionalOptions;
    positionalOptions.add("map", 1);

    po::variables_map options;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), options);
    } catch(const std::exception& e)
    {
        bnw::cerr << "Error: " << e.what() << std::endl;
        bnw::cerr << desc << std::endl;
        return 1;
    }
    po::notify(options);

    if(options.count("help"))
    {
        bnw::cout << desc << std::endl;
        return 0;
    }
    if(options.count("version"))
    {
        bnw::cout  << rttr::version::GetTitle() << " v" << rttr::version::GetVersion() << "-" << rttr::version::GetRevision() << std::endl
        << "Compiled with " << System::getCompilerName() << " for " << System::getOSName() << std::endl;
        return 0;
    }
    if(options.count("map") == 0)
    {
        bnw::cerr << "No map specified" << std::endl;
        return 1;
    }
    if(options.count("ai") == 0)
    {
        bnw::cerr << "No AI specified" << std::endl;
        return 1;
    }

    unsigned random_init = 42;

    RTTRCONFIG.Init();
    RANDOM.Init(random_init);

    const bfs::path mapPath = RTTRCONFIG.ExpandPath(s25::folders::mapsOther) / options["map"].as<std::string>();
    const std::vector<AI::Info> ais = ParseAIOptions(options["ai"].as<std::vector<std::string>>());

    try {
        TrainingGame game(mapPath, ais);
        game.StartReplay("replay.rpl", random_init);
        game.Run(10000);
        game.Close();
        game.SaveGame("savegame.sav");
    } catch (const std::exception& e)
    {
        bnw::cerr <<  e.what() << std::endl;
        return 1;
    }

    bnw::cout << "Game finished" << std::endl;

    return 0;
}
