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

#ifndef CORE_VECTOR_H
#define CORE_VECTOR_H

#include <concepts>
#include <cstdlib>
#include <mdspan/mdspan.hpp>

#include "coordinate.h"

/*!
 * vector field
 * @tparam T numeric type
 */
template <numeric T> class Vector3 {
public:
  /*!
   * Vector3 constructor
   * @param dims field dimensions
   * @param val initial field value
   */
  Vector3(const Coord3<size_t> &dims, const T val) {
    const size_t n = dims.x * dims.y * dims.z;

    x_data = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    y_data = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    z_data = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));

    x = Kokkos::mdspan(x_data, dims.x, dims.y, dims.z);
    y = Kokkos::mdspan(y_data, dims.x, dims.y, dims.z);
    z = Kokkos::mdspan(z_data, dims.x, dims.y, dims.z);

    for (size_t i = 0; i < n; ++i) {
      x_data[i] = val;
      y_data[i] = val;
      z_data[i] = val;
    }
  }

  /*!
   * Vector3 destructor
   */
  ~Vector3() {
    std::free(x_data);
    std::free(y_data);
    std::free(z_data);
  }

  /// x-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> x;

  /// y-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> y;

  /// z-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> z;

private:
  /// x-component data container
  T *x_data;

  /// y-component data container
  T *y_data;

  /// z-component data container
  T *z_data;
};

#endif // CORE_VECTOR_H
