// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "s25py.h"

#include <pybind11/embed.h>

PYBIND11_EMBEDDED_MODULE(s25py, m)
{
    s25py::init_core(m);
}
