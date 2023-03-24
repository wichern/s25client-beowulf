// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/recurrent/RecurrentBase.h"

namespace beowulf {

class GeologistManager : public RecurrentBase
{
public:
    GeologistManager(Beowulf* beowulf);

    void OnRun() override;
};

} // namespace beowulf
