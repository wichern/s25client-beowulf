// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "JoinPlayerInfo.h"
#include "RTTR_Assert.h"
#include "mygettext/mygettext.h"
#include "s25util/Serializer.h"
#include <boost/format.hpp>

JoinPlayerInfo::JoinPlayerInfo() = default;

JoinPlayerInfo::JoinPlayerInfo(const BasePlayerInfo& baseInfo) : PlayerInfo(baseInfo), originName(name) {}

JoinPlayerInfo::JoinPlayerInfo(const PlayerInfo& playerInfo) : PlayerInfo(playerInfo), originName(name) {}

JoinPlayerInfo::JoinPlayerInfo(Serializer& ser)
    : PlayerInfo(ser), originName(ser.PopLongString()), isReady(ser.PopBool())
{}

void JoinPlayerInfo::Serialize(Serializer& ser) const
{
    PlayerInfo::Serialize(ser);
    ser.PushLongString(originName);
    ser.PushBool(isReady);
}

void JoinPlayerInfo::FixSwappedSaveSlot(JoinPlayerInfo& other)
{
    // TODO: This has a code smell.
    // Probably some composition instead of inheritance required?

    // Unswap fixed stuff
    using std::swap;
    swap(originName, other.originName);
    swap(nation, other.nation);
    swap(color, other.color);
    swap(team, other.team);
}

void JoinPlayerInfo::SetAIName(unsigned playerId)
{
    RTTR_Assert(ps == PlayerState::AI);

    switch (aiInfo.type)
    {
        case AI::Type::Dummy:
            name = (boost::format(_("Dummy %u")) % playerId).str();
            break;
        case AI::Type::Beowulf:
            name = (boost::format(_("Beowulf AI %u")) % playerId).str(); break;
        default: 
            name = (boost::format(_("Computer %u")) % playerId).str(); break;
    }

    name += _(" (AI)");

    if(aiInfo.type == AI::Type::Default || aiInfo.type == AI::Type::Beowulf)
    {
        switch(aiInfo.level)
        {
            case AI::Level::Easy: name += _(" (easy)"); break;
            case AI::Level::Medium: name += _(" (medium)"); break;
            case AI::Level::Hard: name += _(" (hard)"); break;
        }
    }
}
