
#include "RttrConfig.h"
#include "files.h"
#include "GlobalGameSettings.h"
#include "world/MapLoader.h"
#include "EventManager.h"
#include "PlayerInfo.h"
#include "Game.h"
#include "GameEvent.h"
#include "ai/AIPlayer.h"
#include "gameTypes/AIInfo.h"
#include "Savegame.h"
#include "factories/AIFactory.h"

#include <boost/filesystem/path.hpp>
#include <iostream>

static PlayerInfo GetPlayer()
{
    PlayerInfo result;
    result.ps = PlayerState::Occupied;
    return result;
}

class TrainingEventManager : public EventManager
{
public:
    TrainingEventManager(unsigned startGF = 0) : EventManager(startGF) {}
    /// Execute the next event increasing the GF to the events GF
    /// If maxGF is given the event is only executed if its GF is <= maxGF and the new GF will be at most maxGF (even if
    /// no event was executed Returns the number of GFs executed
    unsigned ExecuteNextEvent(unsigned maxGF = std::numeric_limits<unsigned>::max())
    {
        if(GetCurrentGF() >= maxGF)
            return 0;
        if(events.empty())
        {
            unsigned numGFs = maxGF - GetCurrentGF();
            currentGF = maxGF;
            return numGFs;
        }
        auto itEvents = events.begin();
        if(itEvents->first > maxGF)
        {
            unsigned numGFs = maxGF - GetCurrentGF();
            currentGF = maxGF;
            return numGFs;
        }
        unsigned numGFs = itEvents->first - GetCurrentGF();
        currentGF = itEvents->first;
        ExecuteEvents(itEvents);
        DestroyCurrentObjects();
        return numGFs;
    }

    /// Return all events of the given object
    std::vector<const GameEvent*> GetObjEvents(const GameObject& obj) const
    {
        std::vector<const GameEvent*> objEvnts;
        for(const auto& event : events)
        {
            for(const GameEvent* ev : event.second)
            {
                if(ev->obj == &obj)
                    objEvnts.push_back(ev);
            }
        }
        return objEvnts;
    }

    /// Check if there is already an event of the given id for this object
    bool IsEventActive(const GameObject& obj, unsigned id) const
    {
        for(const auto& event : events)
        {
            for(const GameEvent* ev : event.second)
            {
                if(ev->id == id && ev->obj == &obj)
                    return true;
            }
        }

        return false;
    }

    /// Remove the event and add a copy that is executed at the given GF
    const GameEvent* RescheduleEvent(const GameEvent* event, unsigned targetGF)
    {
        RemoveEventFromQueue(*event);
        // Hacky but we need to preserve the location (pointer) of the event as objects store it
        const_cast<GameEvent*>(event)->length = targetGF - event->startGF;
        return AddEventToQueue(event);
    }

    std::vector<const GameEvent*> GetEvents() const { return EventManager::GetEvents(); }
};

int main(int argc, char** argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    RTTRCONFIG.Init();


    std::vector<PlayerInfo> playerInfo(2, GetPlayer());

    // @todo: needs shared_ptr?
    std::shared_ptr<Game> game = std::make_shared<Game>(
        GlobalGameSettings(),
        std::make_unique<TrainingEventManager>(),
        playerInfo);
    // em = game->em_
    // world = game->world_
    // ggs = game->ggs_

    const boost::filesystem::path mapPath = RTTRCONFIG.ExpandPath(s25::folders::mapsOther) / "Bergschlumpf.swd";

    //libsiedler2::ArchivItem_Map map;
    MapLoader loader(game->world_);

    if (!loader.Load(mapPath))
    {
        std::cerr << "Could not load " << mapPath << std::endl;
        return 1;
    }

    //if (!loader.PlaceHQs(world, false))
    //    throw std::runtime_error("Could not place HQs");


    std::unique_ptr<AIPlayer> player1(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Hard), 0, game->world_));
    std::unique_ptr<AIPlayer> player2(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Hard), 1, game->world_));

    game->world_.InitAfterLoad();

    unsigned gf = 0;
    while (gf < 10000)
    {
        bool isnfw = static_cast<TrainingEventManager*>(game->em_.get())->GetCurrentGF() % 10;

        if (isnfw) {
            std::cout << gf << std::endl;
            for (gc::GameCommandPtr gc : player1->FetchGameCommands())
                gc->Execute(game->world_, player1->GetPlayerId());
            for (gc::GameCommandPtr gc : player2->FetchGameCommands())
                gc->Execute(game->world_, player2->GetPlayerId());
        }

        player1->RunGF(static_cast<TrainingEventManager*>(game->em_.get())->GetCurrentGF(), isnfw);
        player2->RunGF(static_cast<TrainingEventManager*>(game->em_.get())->GetCurrentGF(), isnfw);


        static_cast<TrainingEventManager*>(game->em_.get())->ExecuteNextEvent(static_cast<TrainingEventManager*>(game->em_.get())->GetCurrentGF() + 1);

        ++gf;
    }

    // Create savegame
    Savegame save;

    for(unsigned i = 0; i < playerInfo.size(); ++i)
        save.AddPlayer(playerInfo[i]);
    save.ggs = game->ggs_;
    save.start_gf = static_cast<TrainingEventManager*>(game->em_.get())->GetCurrentGF();
    save.sgd.debugMode = false;

    //try
    //{
        std::cout << __LINE__ << std::endl;
        save.sgd.MakeSnapshot(*game);
        std::cout << __LINE__ << std::endl;
        return save.Save("savegame.sav", "Bergschlumpf.swd") ? 0 : 1;
    //} catch(std::exception& e)
    //{
    //    return 1;
    //}

    return 0;
}