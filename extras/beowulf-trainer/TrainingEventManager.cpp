// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TrainingEventManager.h"

TrainingEventManager::TrainingEventManager(unsigned startGF)
  : EventManager(startGF)
{

}

unsigned TrainingEventManager::ExecuteNextEvent(unsigned maxGF)
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
