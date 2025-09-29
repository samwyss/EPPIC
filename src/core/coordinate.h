/*
 * Copyright (C) 2025 Samuel Wyss
 *
 * This file is part of EPPIC.
 *
 * EPPIC is free software: you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * EPPIC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EPPIC. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef CORE_COORDINATE_H
#define CORE_COORDINATE_H

#include <concepts>
#include <type_traits>

template <typename T>
concept numeric = std::is_arithmetic_v<T>;

/*!
 * arithmetic container with x, y, and z components
 * @tparam T numeric type
 */
template <numeric T> struct Coord3 {
  T x;
  T y;
  T z;
};

/*!
 * arithmetic container with an x and y component
 * @tparam T numeric type
 */
template <numeric T> struct Coord2 {
  T x;
  T y;
};

/*!
 * arithmetic container with an x component
 * @tparam T numeric type
 */
template <numeric T> struct Coord1 {
  T x;
};

#endif // CORE_COORDINATE_H
