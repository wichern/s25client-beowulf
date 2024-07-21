// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RoadPathFinderDStar.h"
#include "world/MapBase.h"

namespace dstarlite {

Key CalculateKey(const Node& node, unsigned h)
{
    // @todo: h is constant. we could buffer the value or calculate it at the beginning
    //        Also: h is the same for all nodes in this RoadNode. Check how expensive it is to calculate.
    unsigned k2 = std::min(node.g, node.rhs);
    unsigned k1 = k2 + h + node.k_m;
    return {k1, k2};
}

bool operator<(const Key& l, const Key& r)
{
    if(l.k1 < r.k1)
        return true;
    if(l.k1 == r.k2)
        return l.k2 < r.k2;
    return false;
}

} // namespace dstarlite
