// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <vector>


#include "pathfinding/heap.h"

//#define PATHFINDING_OPENLIST
#define PATHFINDING_RADIXHEAP
//#define PATHFINDING_PAIRINGHEAP

class noRoadNode;

struct GetEstimateFromPtr
{
    template<typename T>
    static inline unsigned GetValue(T* el)
    {
        return el->estimate;
    }
};

/// A priority queue based on an unsorted vector with same interface as OpenListPrioQueue
/// Requires a policy that returns the value on which elements should be ordered from the element
/// Note: Order of elements with same value is determined by push/pop operations
template<class T, class T_GetOrderValue = GetEstimateFromPtr>
class OpenListVector : public PriorityHeap
{
    std::vector<T> elements;

public:
    OpenListVector() { elements.reserve(255); }

    T delete_min() override
    {
        RTTR_Assert(!empty());
        const int size = static_cast<int>(elements.size());
        if(size == 1)
        {
            T best = elements.front();
            elements.clear();
            return best;
        }
        int bestIdx = 0;
        unsigned bestEstimate = T_GetOrderValue::GetValue(elements.front());
        for(int i = 1; i < size; i++)
        {
            // Note that this check does not consider nodes with the same value
            // However this is a) correct (same estimate = same quality so no preference from the algorithm)
            // and b) still fully deterministic as the entries are NOT sorted and the insertion-extraction-pattern
            // is completely pre-determined by the graph-structur
            const unsigned estimate = T_GetOrderValue::GetValue(elements[i]);
            if(estimate < bestEstimate)
            {
                bestEstimate = estimate;
                bestIdx = i;
            }
        }
        T best = elements[bestIdx];
        elements[bestIdx] = elements.back();
        elements.pop_back();
        return best;
    }

    void clear() override { elements.clear(); }

    bool empty() const override { return elements.empty(); }

    void insert(T el) override { elements.push_back(el); }

    void decrease_key(T /*el*/) override {}
};
