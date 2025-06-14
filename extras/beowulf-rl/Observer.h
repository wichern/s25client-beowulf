#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

namespace beowulf {

/// @brief Observe the training
class Observer
{
public:
    // Print the current state:
    //
    // current episode      wall clock
    // ASCII map
    void print();
};

} // namespace beowulf
