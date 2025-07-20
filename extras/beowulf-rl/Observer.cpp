#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Observer.h"
#include "AsciiMap.h"
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

unsigned nextResultDirId();

inline std::string repeat(const std::string& in, const size_t count)
{
    std::string ret;
    for (size_t i = 0u; i < count; ++i)
        ret += in;
    return ret;
}

Observer::Observer(unsigned explorationSteps, unsigned maxGf)
    : explorationSteps_(explorationSteps)
    , maxGf_(maxGf)
    , trainingStart_(std::chrono::steady_clock::now())
{
    // Create output directory for reports
    outDir_ = bfs::current_path() / std::string(BEOWULF_REPORT_DIR_PREFIX + std::to_string(nextResultDirId()));
    bfs::create_directory(outDir_);

    outResultsCsv_.open(outDir_ / "results.csv", std::ofstream::out | std::ofstream::trunc);
    outResultsCsv_ << "Reward;Epsilon\n";

    live.lastFrame_ = trainingStart_ - std::chrono::seconds(1);
}

void Observer::BeginExplorationRun()
{
    state_ = TrainingState::Exploration;
}

void Observer::EndExplorationRun(size_t explorationSteps)
{
    currentExplorationStep_ = explorationSteps;
}

void Observer::BeginTrainingRun()
{
    state_ = TrainingState::Training;
    live.currentEpisode_++;
}

void Observer::EndTrainingRun()
{

}

void Observer::BeginTestRun()
{
    state_ = TrainingState::Test;
    testRun_++;

    testrun.episodeDir_ = outDir_ / "testruns" / std::to_string(testRun_);
    bfs::create_directories(testrun.episodeDir_);

    RTTR_Assert(!testrun.outEpisodeActions_.is_open());
    testrun.outEpisodeActions_.open(testrun.episodeDir_ / "actions.txt", std::ofstream::out | std::ofstream::trunc);

    RTTR_Assert(!testrun.outQValues_.is_open());
    testrun.outQValues_.open(testrun.episodeDir_ / "qvalues.txt", std::ofstream::out | std::ofstream::trunc);
    testrun.outQValues_ << std::fixed << std::setprecision(4);
}

void Observer::OnSetBuildingSite(const MapPoint& pt, BuildingType bld)
{
    if (testrun.outEpisodeActions_.is_open())
        testrun.outEpisodeActions_ << currentGf_ << " " << pt << " " << bld << "\n";
}

void Observer::OnQValues(const arma::colvec& qvalues)
{
    if (testrun.outQValues_.is_open()) {
        for (double val : qvalues)
            testrun.outQValues_ << val << " ";
        testrun.outQValues_ << "\n";
    }
}

void Observer::EndTestRun(double reward, double epsilon, const GameWorld& gwb)
{
    testrun.outEpisodeActions_.close();
    testrun.outQValues_.close();

    live.rewards_.push_back(reward);
    if (live.rewards_.size() > 50)
        live.rewards_.erase(live.rewards_.begin());

    live.epsilons_.push_back(epsilon * 100.0);
    if (live.epsilons_.size() > 50)
        live.epsilons_.erase(live.epsilons_.begin());

    // Create AsciiMap
    AsciiMap amap(gwb);
    for (unsigned i = 0u; i < gwb.GetNumPlayers(); ++i)
        amap.drawPlayer(i);
    boost::filesystem::ofstream amapOut(testrun.episodeDir_ / "map.txt", std::ofstream::out | std::ofstream::trunc);
    amap.write(amapOut);

    // Add reward and epsilon to results.csv
    outResultsCsv_ << reward << ";" << epsilon << "\n" << std::flush;
}

void Observer::Print(const GameWorld& gwb, unsigned playerId)
{
    // ANSI colors
    static const char* RESET = "\033[0m";
    static const char* BLUE = "\033[34m";
    static const char* GREEN = "\033[32m";
    //static const char* RED = "\033[31m";
    //static const char* YELLOW = "\033[33m";

    auto now = std::chrono::steady_clock::now();
    if (live.lastFrame_ + std::chrono::milliseconds(250) > now)
        return;
    live.lastFrame_ = now;

    // Move cursor up and clear
    printConsole("\033[H\033[J");

    // Get terminal dimensions
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    unsigned short width = std::min((unsigned short)120, w.ws_col);

    // top
    printConsole("+%s+\n", std::string(width - 2, '-').c_str());

    // title with episode and elapsed time
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - trainingStart_);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(now - trainingStart_);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(now - trainingStart_);
    switch (state_) {
    case TrainingState::Pretraining:
    {
        std::string padding = std::string(width - 28, ' ');
        printConsole("| %sPre-Training Phase%s%s%02ld:%02ld:%02ld |\n",
            BLUE, RESET,
            padding.c_str(),
            hours.count(), minutes.count() % 60, seconds.count() % 60);
    } break;
    case TrainingState::Exploration:
    {
        std::string padding = std::string(width - 29, ' ');
        printConsole("| %sExploration Phase%s%s%02ld:%02ld:%02ld |\n",
            BLUE, RESET,
            padding.c_str(),
            hours.count(), minutes.count() % 60, seconds.count() % 60);
    } break;
    case TrainingState::Training:
    case TrainingState::Test:
    {
        std::string padding = std::string(width - 29, ' ');
        printConsole("| %sEpisode%s %-*u%s %02ld:%02ld:%02ld |\n",
            BLUE, RESET,
            8, live.currentEpisode_,
            padding.c_str(),
            hours.count(), minutes.count() % 60, seconds.count() % 60);
    } break;
    }

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

    // horizontal line
    unsigned leftWidth = width/ 2;
    unsigned rightWidth = width - leftWidth - 1;
    printConsole("+%s+%s+\n",
        std::string(leftWidth - 1, '-').c_str(),
        std::string(rightWidth - 1, '-').c_str());

    // charts
    static const size_t chartHeight = 8;
    std::vector<std::string> linesReward = printChart(live.rewards_, chartHeight, leftWidth - 3);
    std::vector<std::string> linesEpsilon = printChart(live.epsilons_, chartHeight, rightWidth - 3);
    std::string padding_reward = std::string(leftWidth - 9, ' ');
    std::string padding_epsilon = std::string(rightWidth - 10, ' ');
    printConsole("| %sReward%s%s | %sEpsilon%s%s |\n",
        BLUE, RESET,
        padding_reward.c_str(),
        BLUE, RESET,
        padding_epsilon.c_str());
    for (size_t i = 0; i < chartHeight; ++i)
    {
        printConsole("| %s | %s |\n",
            linesReward[i].c_str(),
            linesEpsilon[i].c_str()
        );
    }

    // horizontal line
    printConsole("+%s+%s+\n",
        std::string(leftWidth - 1, '-').c_str(),
        std::string(rightWidth - 1, '-').c_str());

    // print a part of the test run result
    auto const& player = gwb.GetPlayer(playerId);
    AsciiMap debug(gwb, player.GetHQPos(), (width)/4, (width)/8, 1, AsciiMap::Border::ASCII);
    debug.drawPlayer(playerId);
    debug.write();
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

unsigned nextResultDirId()
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

} // namespace beowulf
