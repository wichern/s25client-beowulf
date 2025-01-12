// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

class AIWorld : public MapBase
{
public:
    AIWorld(bool fow);

public:
    // Roads
    bool HasRoad(const MapPoint& pt, Direction dir) const;
};