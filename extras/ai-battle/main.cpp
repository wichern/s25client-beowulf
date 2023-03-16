// Copyright (C) 2005 - 2023 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/AIPlayer.h"
#include "AsyncChecksum.h"
#include "EventManager.h"
#include "factories/AIFactory.h"
#include "Game.h"
#include "GlobalGameSettings.h"
#include "PlayerInfo.h"
#include "RttrConfig.h"
#include "ReplayInfo.h"
#include "files.h"
#include "network/PlayerGameCommands.h"
#include "gameTypes/MapInfo.h"
#include "gameTypes/Nation.h"
#include "s25util/colors.h"
#include "random/Random.h"
#include "world/GameWorld.h"
#include "world/MapLoader.h"

#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <vector>
#include <iomanip>

#ifndef _MSC_VER
#    include <csignal>
#else
#    include <windows.h>
#endif

namespace bfs = boost::filesystem;
namespace bnw = boost::nowide;
namespace po = boost::program_options;

volatile sig_atomic_t stop = false;

void InstallSignalHandlers();

#ifdef _WIN32
BOOL WINAPI ConsoleSignalHandler(DWORD dwCtrlType);
#else
void ConsoleSignalHandler(int signum);
#endif // _WIN32

bool parseArgs(int argc, char** argv, std::string& mapPath, std::vector<std::string>& players, bool& replay);

/**
 *  Main
 *
 *  @param[in] argc Argument count
 *  @param[in] argv Arguments
 *
 *  @return Exit status
 * 
 * @todo: CTRL-C will stop the game and store the replay.
 * @todo: Write a program that shows replays only
 */
int main(int argc, char** argv)
{
    std::string mapPath;
    std::vector<std::string> players;
    bool replay;
    if (!parseArgs(argc, argv, mapPath, players, replay))
    {
        return EXIT_FAILURE;
    }

    InstallSignalHandlers();

    bnw::cout << "MAP: " << mapPath << "\n";
    bnw::cout << "Players:\n";
    for (const auto& player : players)
    {
        bnw::cout << "- " << player << "\n";
    }

    // LOG.setLogFilepath(bfs::current_path());
    // Initialize random generator
    unsigned random_init = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    RANDOM.Init(random_init);

    if(!RTTRCONFIG.Init())
    {
        bnw::cerr << "RTTRCONFIG.Init() failed\n";
        return EXIT_FAILURE;
    }

    GlobalGameSettings ggs;
    ggs.exploration = Exploration::Classic;

    std::vector<PlayerInfo> playerInfos;
    for (const auto& player : players)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.name = player + " #" + std::to_string(playerInfos.size());
        pi.nation = Nation::Romans;
        pi.color = PLAYER_COLORS[playerInfos.size()];
        pi.team = Team::None;
        pi.aiInfo.level = AI::Level::Hard;

        if (player == "AIJH")
        {
            pi.aiInfo.type = AI::Type::Default;
        }
        else if (player == "Dummy")
        {
            pi.aiInfo.type = AI::Type::Dummy;
        }
        else
        {
            bnw::cerr << "Invalid AI player name: " << player << "\n";
            return EXIT_FAILURE;
        }
        
        playerInfos.push_back(pi);
    }

    Game game(ggs, 0 /* start-frame */, playerInfos);

    MapLoader mapLoader(game.world_);
    if(!mapLoader.Load(mapPath))
    {
        bnw::cerr << "Could not load map: " << mapPath << "\n";
        return EXIT_FAILURE;
    }

    for (unsigned i = 0; i < playerInfos.size(); ++i)
        game.AddAIPlayer(AIFactory::Create(playerInfos[i].aiInfo, i, game.world_));

    std::unique_ptr<ReplayInfo> replayInfo;
    if (replay)
    {
        replayInfo = std::make_unique<ReplayInfo>();
        replayInfo->filename = "ai-battle " + s25util::Time::FormatTime("%Y-%m-%d_%H-%i-%s") + ".rpl";
        replayInfo->replay.random_init = random_init;
        replayInfo->replay.ggs = ggs;

        for(const auto& pi : playerInfos)
            replayInfo->replay.AddPlayer(pi);
    }
    
    MapInfo mapInfo;
    mapInfo.type = MapType::OldMap;
    mapInfo.title = "AI Battle";  // @todo: check GameServer.cpp:148 on how to set title
    mapInfo.filepath = mapPath;

    if(!mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum))
    {
        bnw::cerr << "Could not load map data from " << mapPath << "\n";
        return EXIT_FAILURE;
    }
    bfs::path luaFilePath = bfs::path(mapInfo.filepath).replace_extension("lua");
    if(bfs::is_regular_file(luaFilePath))
    {
        if(!mapInfo.luaData.CompressFromFile(luaFilePath, &mapInfo.luaChecksum))
        {
            bnw::cerr << "Could not load lua data from " << mapPath << "\n";
            return EXIT_FAILURE;
        }
        mapInfo.luaFilepath = luaFilePath;
    }

    if(!mapInfo.verifySize())
    {
        bnw::cerr << "Map is too large: " << mapPath << "\n";
        return EXIT_FAILURE;
    }

    if (replayInfo && !replayInfo->replay.StartRecording(RTTRCONFIG.ExpandPath(s25::folders::replays) / replayInfo->filename, mapInfo))
    {
        bnw::cerr << "Could not start recording replay\n";
        return EXIT_FAILURE;
    }
    
    game.Start(false /* startFromSave */);
    while(!game.IsGameFinished() && !stop)
    {
        bool isnwf = (game.em_->GetCurrentGF() % 5 == 0);
        for (unsigned player = 0; player < playerInfos.size(); ++player) {
            game.GetAIPlayer(player)->RunGF(game.em_->GetCurrentGF(), isnwf);
            
            auto gcs = game.GetAIPlayer(player)->FetchGameCommands();
            for(gc::GameCommandPtr& gc : gcs)
                gc->Execute(game.world_, player);
            if(replayInfo && !gcs.empty() && replayInfo->replay.IsRecording()) {
                PlayerGameCommands playergcs;
                playergcs.gcs = gcs;
                replayInfo->replay.AddGameCommand(game.em_->GetCurrentGF(), player, playergcs);
            }
        }

        game.RunGF();
        
        if (game.em_->GetCurrentGF() % 500 == 0) {
            bnw::cout << "GF " << game.em_->GetCurrentGF() << "\n";
            for (unsigned i = 0; i < playerInfos.size(); ++i)
                bnw::cout << playerInfos[i].name 
                           << ": Country: " << std::setw(5) << game.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Country)
                           << ", Buildings: " << std::setw(3) << game.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Buildings)
                           << ", Military: " << std::setw(3) << game.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Military)
                           << ", Gold: " << std::setw(3) << game.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Gold)
                           << ", Productivity: " << std::setw(2) << game.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Productivity)
                           << "\n";
        }
    }

    if(replayInfo && replayInfo->replay.IsRecording())
        replayInfo->replay.UpdateLastGF(game.em_->GetCurrentGF());

    if(replayInfo)
    {
        if(replayInfo->replay.IsRecording())
            replayInfo->replay.StopRecording();
        replayInfo->replay.Close();
        bnw::cout << "Replay written to " << replayInfo->filename << "\n";
        replayInfo.reset();
    }

    return EXIT_SUCCESS;
}

bool parseArgs(int argc, char** argv, std::string& mapPath, std::vector<std::string>& players, bool& replay)
{
    bnw::args _(argc, argv);

    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help,h", "Show help")
        ("replay", "Record replay")
        ("map,m", po::value<std::string>(),"Map to load")
        ("player", po::value<std::vector<std::string>>(), "Player configurations")
        ;
    // clang-format on
    po::positional_options_description positionalOptions;
    positionalOptions.add("map", 1);
    positionalOptions.add("player", 1);

    po::variables_map options;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions).run(), options);
        // Catch the generic stdlib exception as hidden visibility messes up boost typeinfo on OSX
    } catch(const std::exception& e)
    {
        bnw::cerr << "Error: " << e.what() << "\n\n";
        bnw::cerr << desc << "\n";
        return false;
    }
    po::notify(options);

    if(options.count("help"))
    {
        bnw::cout << desc << "\n";
        std::exit(EXIT_SUCCESS);
    }

    if(!options.count("map"))
    {
        bnw::cerr << "No map file specified.\n";
        return false;
    }

    if(!options.count("player"))
    {
        bnw::cerr << "No players specified.\n";
        return false;
    }

    mapPath = options["map"].as<std::string>();
    players = options["player"].as<std::vector<std::string>>();
    replay = options.count("replay");

    return true;
}

#ifdef _WIN32
BOOL WINAPI ConsoleSignalHandler(DWORD dwCtrlType)
{
    switch(dwCtrlType)
    {
        case CTRL_C_EVENT:
        {
            stop = true;
            return TRUE;
        }
        break;
    }
    return FALSE;
}
#else
void ConsoleSignalHandler(int /*signum*/)
{
    bnw::cout << "stopping...\n" << std::flush;
    stop = true;
}
#endif

void InstallSignalHandlers()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleSignalHandler, TRUE);
#else
    struct sigaction sa;
    sa.sa_handler = ConsoleSignalHandler;
    sa.sa_flags = 0; // SA_RESTART would not allow to interrupt connect call;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
#endif
}
