// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef ADDONCHANGEGOLDDEPOSITS_H_INCLUDED
#define ADDONCHANGEGOLDDEPOSITS_H_INCLUDED

#pragma once

#include "AddonList.h"
#include "mygettext/src/mygettext.h"

/**
 *  Addon for changing gold deposits to other resources or
 *  to remove them completely
 */
class AddonChangeGoldDeposits : public AddonList
{
public:
    AddonChangeGoldDeposits()
        : AddonList(AddonId::CHANGE_GOLD_DEPOSITS, ADDONGROUP_MILITARY | ADDONGROUP_ECONOMY, _("Change gold deposits"),
                    _("You can remove gold resources completely or convert them into iron ore, coal or granite.\n\n"
                      "You'll probably want to convert gold to iron ore, as this (on most maps)\n"
                      "allows you to utilize the additional coal not needed for minting anymore."),
                    0)
    {
        addOption(_("No change"));
        addOption(_("Remove gold completely"));
        addOption(_("Convert to iron ore"));
        addOption(_("Convert to coal"));
        addOption(_("Convert to granite"));
    }
};

#endif // !ADDONCHANGEGOLDDEPOSITS_H_INCLUDED
