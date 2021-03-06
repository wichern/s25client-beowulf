// Copyright (c) 2020 - 2020 Settlers Freaks (sf-team at siedler25.org)
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

#pragma once

#include "MaxEnumValue.h"
#include "enum_cast.hpp"

namespace helpers {

/// Array that is meant to be index with enum values instead of integrals
/// <typeparam name="T_Elements">Type of the elements</typeparam>
/// <typeparam name="T_Index">Type of the indices</typeparam>
template<typename T_Elements, typename T_Index>
struct EnumArray
{
    using value_type = T_Elements;
    using iterator = T_Elements*;
    using const_iterator = const T_Elements*;

    constexpr T_Elements* data() { return elems; }
    constexpr const T_Elements* data() const { return elems; }
    static constexpr unsigned size() { return helpers::MaxEnumValue_v<T_Index> + 1u; }

    T_Elements& operator[](T_Index idx) noexcept { return elems[rttr::enum_cast(idx)]; }
    constexpr const T_Elements& operator[](T_Index idx) const noexcept { return elems[rttr::enum_cast(idx)]; }

    iterator begin() noexcept { return elems; }
    constexpr const_iterator begin() const noexcept { return elems; }
    iterator end() noexcept { return elems + size(); }
    constexpr const_iterator end() const noexcept { return elems + size(); }

    T_Elements elems[size()];
};
} // namespace helpers
