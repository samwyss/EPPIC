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
template <numeric T> class Scalar3 {
public:
  /*!
   * Scalar3 constructor
   * @param dims field dimensions
   * @param val initial field value
   */
  Scalar3(const Coord3<ui_t> &dims, const T val) {
    const ui_t n = dims.x * dims.y * dims.z;

    data_arr = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    data = Kokkos::mdspan(data_arr, dims.x, dims.y, dims.z);

    for (ui_t i = 0; i < n; ++i) {
      data_arr[i] = val;
    }
  }

  /*!
   * Scalar destructor
   */
  ~Scalar3() { std::free(data_arr); }

  /// data view
  Kokkos::mdspan<T, Kokkos::dextents<ui_t, 3>> data;

private:
  /// data container
  T *data_arr;
};

#endif // CORE_SCALAR_H
