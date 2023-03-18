// Copyright (C) 2005 - 2023 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PlayerInfo.h"
#include "gameTypes/Nation.h"
#include "s25util/colors.h"

#include "AIGameManager.h"

#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <vector>

#ifndef _MSC_VER
#    include <csignal>
#else
#    include <windows.h>
#endif

namespace bnw = boost::nowide;
namespace po = boost::program_options;

volatile sig_atomic_t g_stop = false;

void InstallSignalHandlers();

#ifdef _WIN32
BOOL WINAPI ConsoleSignalHandler(DWORD dwCtrlType);
#else
void ConsoleSignalHandler(int signum);
#endif // _WIN32

/**
 * Parse and validate command line arguments.
 *
 * Note: will call std::exit() when '-h' was given.
 *
 * @param[in]   argc    Arguement count
 * @param[in]   argv    Arguments
 * @param[out]  mapPath Path to map file
 * @param[out]  players List of AI players
 * @param[out]  replay  whether replay shall be generated
 * @return              true when input validation succeeded
 *
 * @todo: Use GUI mode by default
 * @todo: Add detailed description to argument parser
 */
bool parseArgs(int argc, char** argv, std::string& mapPath, std::vector<std::string>& players, bool& replay);

/**
 *  Main
 *
 *  @param[in] argc Argument count
 *  @param[in] argv Arguments
 *
 *  @return Exit status
 *
 * CTRL-C will stop the game and store the replay.
 */
int main(int argc, char** argv)
{
    std::string mapPath;
    std::vector<std::string> players;
    bool replay;
    if(!parseArgs(argc, argv, mapPath, players, replay))
    {
        return EXIT_FAILURE;
    }

    InstallSignalHandlers();

    bnw::cout << "MAP: " << mapPath << "\n";
    bnw::cout << "Players:\n";
    for(const auto& player : players)
    {
        bnw::cout << "- " << player << "\n";
    }

    std::vector<PlayerInfo> playerInfos;
    for(const auto& player : players)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.name = player + " #" + std::to_string(playerInfos.size());
        pi.nation = Nation::Romans;
        pi.color = PLAYER_COLORS[playerInfos.size()];
        pi.team = Team::None;
        pi.aiInfo.level = AI::Level::Hard;

        if(player == "AIJH")
        {
            pi.aiInfo.type = AI::Type::Default;
        } else if(player == "Dummy")
        {
            pi.aiInfo.type = AI::Type::Dummy;
        } else
        {
            bnw::cerr << "Invalid AI player name: " << player << "\n";
            return EXIT_FAILURE;
        }

        playerInfos.push_back(pi);
    }

    AIGameManager gameManager(replay, std::move(playerInfos));
    if(!gameManager.Start(mapPath))
        return EXIT_FAILURE;

    while(!g_stop)
    {
        if(!gameManager.Run())
            break;
    }

    gameManager.Stop();

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
            g_stop = true;
            return TRUE;
        }
        break;
    }
    return FALSE;
}
#else
void ConsoleSignalHandler(int /*signum*/)
{
    g_stop = true;
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
