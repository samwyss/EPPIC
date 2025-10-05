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

#ifndef CORE_SCALAR_H
#define CORE_SCALAR_H

#include <concepts>
#include <cstdlib>
#include <mdspan/mdspan.hpp>

#include "coordinate.h"
#include "type.h"

/*!
 * 3D scalar field
 * @tparam T numeric type
 */
template <numeric T> struct Scalar3 {
  /// data view
  Kokkos::mdspan<T, Kokkos::dextents<ui_t, 3>> v = Kokkos::mdspan(data, 0, 0, 0);

  /// data container
  T *data = nullptr;

  /*!
   * initializes Scalar3
   * @param dims field dimensions
   * @param val initial field value
   */
  void init(const Coord3<ui_t> &dims, const T val) noexcept {
    const ui_t n = dims.x * dims.y * dims.z;

    data = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    v = Kokkos::mdspan(data, dims.x, dims.y, dims.z);

    for (ui_t i = 0; i < n; ++i) {
      data[i] = val;
    }
  }

  /*!
   * resets Scalar3 to default state
   * @note this frees existing data and resets dataview
   */
  void reset() noexcept {
    std::free(data);
    data = nullptr;
    v = Kokkos::mdspan(data, 0, 0, 0);
  }
};

#endif // CORE_SCALAR_H
