// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <vector>

class noRoadNode;

/// @brief Radix Heap
/// A radix heap has the following limitations:
///     1. can only use unsigned integers as keys
///     2. After the first delete_min(), can only insert or decrease keys bigger than the current minimum.
class RadixHeap
{
    /// Buckets
    std::array<std::vector<const noRoadNode*>, 33> b;

    /// Bucket bounds
    std::array<unsigned, 33> u;

    unsigned size_;

public:
    RadixHeap() { clear(); }

    const noRoadNode* delete_min();
    void clear();
    inline bool empty() const { return size_ == 0; }
    void insert(const noRoadNode* el);
    void decrease_key(const noRoadNode* el);

private:
    inline unsigned get_bucket(unsigned key, unsigned start = 32) const
    {
        unsigned i = start;
        while(u[i] > key)
            i--;
        return i;
    }
};
