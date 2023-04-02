// Copyright (C) 2005 - 2023 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class noRoadNode;

class PriorityHeap
{
public:
    virtual void clear() = 0;
    virtual void insert(const noRoadNode* node) = 0;
    virtual const noRoadNode* delete_min() = 0;
    virtual void decrease_key(const noRoadNode* node) = 0;
    virtual bool empty() const = 0;
};