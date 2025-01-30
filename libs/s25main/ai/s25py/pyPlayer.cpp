// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyPlayer.h"

#include "ai/AIPlayer.h"
#include "buildings/nobHQ.h"
#include "buildings/nobHarborBuilding.h"

namespace s25py {

PyPlayer::PyPlayer() {}

PyPlayer::~PyPlayer() {}

void PyPlayer::RunGF(unsigned gf, bool gfisnwf)
{
    RTTR_UNUSED(gf);
    RTTR_UNUSED(gfisnwf);
}

void PyPlayer::OnChatMessage(unsigned sendPlayerId, ChatDestination dest, const std::string& msg)
{
    RTTR_UNUSED(sendPlayerId);
    RTTR_UNUSED(dest);
    RTTR_UNUSED(msg);
}

std::vector<PyBuilding> PyPlayer::GetHeadquaters() const
{
    std::vector<PyBuilding> ret;

    const nobHQ* hq = player_->getAIInterface().GetHeadquarter();
    if(hq)
        ret.push_back({hq});

    for(const nobHarborBuilding* harbor : player_->getAIInterface().GetHarbors())
        ret.push_back({harbor});

    return ret;
}

} // namespace s25py