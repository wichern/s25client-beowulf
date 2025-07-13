#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Observer.h"
#include "AsciiMap.h"
#include "Environment.h"
#include "world/GameWorld.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>
#include <cmath>
#include <boost/nowide/iostream.hpp>
#include <boost/filesystem.hpp>
#include "PointOutput.h"
#include "gameTypes/GameTypesOutput.h"

#ifdef WIN32
#    include "Windows.h"
#endif

static const char* BEOWULF_REPORT_DIR_PREFIX = "beowulf-rl.";

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;

namespace beowulf {

#if defined(__MINGW32__) && !defined(__clang__)
void printConsole(const char* fmt, ...) __attribute__((format(gnu_printf, 1, 2)));
#elif defined __GNUC__
void printConsole(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#else
void printConsole(const char* fmt, ...);
#endif

Observer& Observer::getInstance()
{
    static Observer instance;
    return instance;
}

void Observer::init(unsigned explorationSteps, unsigned maxGf, beowulf::Environment* env)
{
    const auto cwd = bfs::current_path();
    outDir_ = cwd / std::string(BEOWULF_REPORT_DIR_PREFIX + std::to_string(NextResultDirId()));
    bfs::create_directory(outDir_);

    explorationSteps_ = explorationSteps;
    maxGf_ = maxGf;
    env_ = env;
    trainingStart_ = std::chrono::steady_clock::now();
    lastFrame_ = trainingStart_ - std::chrono::seconds(1);
}

void Observer::addExplorationResult(unsigned steps)
{
    currentExplorationStep_ = steps;
}

void Observer::addEpisodeResult(double reward, double epsilon)
{
    currentExplorationStep_ = explorationSteps_;

    // ignore the huge negative rewards from losses
    //rewards_.push_back(std::max(reward, 0.0));
    rewards_.push_back(reward);
    if (rewards_.size() > 50)
        rewards_.erase(rewards_.begin());

    epsilons_.push_back(epsilon * 100.0);
    if (epsilons_.size() > 50)
        epsilons_.erase(epsilons_.begin());

    setCurrentGf(maxGf_);

    // write asciimap

    // Create file with all actions of the agent
    // Create replay
    // Print Reward
    // all Q-values from all states into additional csv-file (to check for exploding or near zero values)
}

inline std::string repeat(const std::string& in, size_t count)
{
    std::string ret;
    for (size_t i = 0u; i < count; ++i)
        ret += in;
    return ret;
}

void Observer::printState()
{
    // ANSI colors
    static const char* RESET = "\033[0m";
    static const char* BLUE = "\033[34m";
    static const char* GREEN = "\033[32m";
    //static const char* RED = "\033[31m";
    //static const char* YELLOW = "\033[33m";

    auto now = std::chrono::steady_clock::now();
    if (lastFrame_ + std::chrono::milliseconds(500) > now)
        return;
    lastFrame_ = now;

    // Move cursor up and clear
    printConsole("\033[%uA\033[J", lastHeight_);
    lastHeight_ = 0u;

    // Get terminal dimensions
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    unsigned short width = std::min((unsigned short)120, w.ws_col);
    unsigned short height = w.ws_row;

    // top
    printConsole("+%s+\n", std::string(width - 2, '-').c_str());
    lastHeight_++;

    // title with episode and elapsed time
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - trainingStart_);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(now - trainingStart_);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(now - trainingStart_);
    if (currentExplorationStep_ <= explorationSteps_) {
        std::string padding = std::string(width - 29, ' ');
        printConsole("| %sExploration Phase%s%s%02ld:%02ld:%02ld |\n",
            BLUE, RESET,
            padding.c_str(),
            hours.count(), minutes.count() % 60, seconds.count() % 60);
        } else {
    std::string padding = std::string(width - 29, ' ');
            printConsole("| %sEpisode%s %-*u%s %02ld:%02ld:%02ld |\n",
                BLUE, RESET,
                8, currentEpisode_,
                padding.c_str(),
                hours.count(), minutes.count() % 60, seconds.count() % 60);
        }
    lastHeight_++;

    // progress bar
    unsigned barWidth = width - 11;
    float percent = static_cast<float>(currentGf_) / static_cast<float>(maxGf_);
    if (currentExplorationStep_ <= explorationSteps_)
        percent = static_cast<float>(currentExplorationStep_) / static_cast<float>(explorationSteps_);
    unsigned filled = static_cast<int>(percent * static_cast<float>(barWidth));
    printConsole("| %s[%s%s] %3d%%%s |\n",
        GREEN,
        std::string(filled, '#').c_str(),
        std::string(barWidth - filled, '-').c_str(),
        static_cast<int>(percent * 100.0),
        RESET);
    lastHeight_++;

    // horizontal line
    unsigned leftWidth = width/ 2;
    unsigned rightWidth = width - leftWidth - 1;
    printConsole("+%s+%s+\n",
        std::string(leftWidth - 1, '-').c_str(),
        std::string(rightWidth - 1, '-').c_str());
    lastHeight_++;

    // charts
    static const size_t chartHeight = 8;
    std::vector<std::string> linesReward = printChart(rewards_, chartHeight, leftWidth - 3);
    std::vector<std::string> linesEpsilon = printChart(epsilons_, chartHeight, rightWidth - 3);
    std::string padding_reward = std::string(leftWidth - 9, ' ');
    std::string padding_epsilon = std::string(rightWidth - 10, ' ');
    printConsole("| %sReward%s%s | %sEpsilon%s%s |\n",
        BLUE, RESET,
        padding_reward.c_str(),
        BLUE, RESET,
        padding_epsilon.c_str());
    lastHeight_++;
    for (size_t i = 0; i < chartHeight; ++i)
    {
        printConsole("| %s | %s |\n",
            linesReward[i].c_str(),
            linesEpsilon[i].c_str()
        );
        lastHeight_++;
    }

    static const unsigned minMapHeight = 10;

    if ((lastHeight_ + minMapHeight + 1) < height && env_ && env_->world_)
    {
        // horizontal line
        printConsole("+%s+%s+\n",
            std::string(leftWidth - 1, '-').c_str(),
            std::string(rightWidth - 1, '-').c_str());
        lastHeight_++;

        // print a part of the test run result
        auto const& world = *(env_->world_);
        auto const& player = world.GetPlayer(env_->agentId_);
        AsciiMap debug(world, player.GetHQPos(), (width)/4, (width)/8, 1, AsciiMap::Border::ASCII);
        debug.drawPlayer(env_->agentId_);
        debug.write();
        lastHeight_ += debug.h_;
    } else {
        // horizontal line
        printConsole("+%s+%s+\n",
            std::string(leftWidth - 1, '-').c_str(),
            std::string(rightWidth - 1, '-').c_str());
        lastHeight_++;
    }
}

std::vector<std::string> Observer::printChart(const std::vector<double> values, unsigned height, unsigned width) const
{
    std::vector<std::string> lines(height, std::string(width, ' '));

    if (values.empty())
        return lines;

    // normalize
    double maxVal = *std::max_element(values.begin(), values.end());
    double minVal = *std::min_element(values.begin(), values.end());
    std::vector<int> normalized;
    for (double d : values) {
        int val = (maxVal - minVal < 1e-6) ? height / 2
                                           : int(((d - minVal) / (maxVal - minVal)) * (height - 1));
        normalized.push_back(val);
    }

    unsigned step = std::max(1u, static_cast<unsigned>(values.size()) / width);
    for (unsigned i = 0, x = 0; i < values.size() && x+6 < width; i += step, ++x) {
        lines[height - 1 - normalized[i]][6 + x] = '*';
    }
    for (unsigned i = 0; i < height; ++i) {
        double val = minVal + (maxVal - minVal) * (height - 1 - i) / (height - 1);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%5.1f:", val);
        lines[i].replace(0, 6, buffer);
    }
    return lines;
}

void printConsole(const char* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    const int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if(len > 0 && (size_t)len < sizeof(buffer))
    {
#ifdef WIN32
        static auto h = setupStdOut();
        WriteConsoleA(h, buffer, len, 0, 0);
#else
        bnw::cout << buffer;
#endif
    }
}

unsigned Observer::NextResultDirId() const
{
    unsigned ret = 0;

    for (const auto& it : bfs::directory_iterator(bfs::current_path())) {
        if (bfs::is_directory(it.status())) {
            std::string dirname = it.path().filename().string();
            if (dirname.rfind(BEOWULF_REPORT_DIR_PREFIX) != 0)
                continue;

            // extract ID
            std::string prefix(BEOWULF_REPORT_DIR_PREFIX);
            if (!dirname.compare(0, prefix.size(), prefix))
                ret = std::max(ret, static_cast<unsigned>(std::stoul(dirname.substr(prefix.size()))));
        }
    }

    return ret + 1;
}

void Observer::beginEpisode()
{
    if (outEpisodeActions_.is_open())
        outEpisodeActions_.close();
    currentEpisode_++;
    episodeDir_ = outDir_ / std::to_string(currentEpisode_);
    bfs::create_directory(episodeDir_);
    outEpisodeActions_.open(episodeDir_ / "actions.txt", std::ofstream::out | std::ofstream::trunc);
}

void Observer::storeSetBuildingSite(const MapPoint& pt, BuildingType bld)
{
    outEpisodeActions_ << pt << " : " << bld << "\n";
}

} // namespace beowulf
