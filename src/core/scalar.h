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
#include <expected>
#include <fmt/format.h>
#include <mdspan/mdspan.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "coordinate.h"
#include "type.h"

/*!
 * 3D scalar field
 * @tparam T numeric type
 */
template <numeric T> struct Scalar3 {
  /// data view
  Kokkos::mdspan<T, Kokkos::dextents<ui_t, 3>> v;

  /// data container
  T *data = nullptr;

  /*!
   * initializes Scalar3
   * @param dims field dimensions
   * @param val initial field value
   * @return std::expected<void, std::string> for {success, error} cases respectively
   */
  [[nodiscard]] std::expected<void, std::string> init(const Coord3<ui_t> &dims, const T val) noexcept {
    SPDLOG_TRACE("enter Scalar3::init");

    const ui_t n = dims.x * dims.y * dims.z;

    if (n * sizeof(T) % 64 != 0) {
      const auto error =
          fmt::format("number of elements `{}` cannot be aligned to 64 byte boundary for type {}", n, type_name<T>());
      SPDLOG_CRITICAL(error);
      return std::unexpected(error);
    }

    try {
      data = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    } catch (const std::bad_alloc &err) {
      const auto error = fmt::format("unable to allocate memory for `data` with `{}` elements ({} bytes): {}", n,
                                     n * sizeof(T), err.what());
      SPDLOG_CRITICAL(error);
      return std::unexpected(error);
    }

    v = Kokkos::mdspan(data, dims.x, dims.y, dims.z);

    for (ui_t i = 0; i < n; ++i) {
      data[i] = val;
    }

    SPDLOG_TRACE("exit Scalar3::init");
    return {};
  }

  /*!
   * resets Scalar3 to default state
   * @note this frees existing data and resets dataview
   */
  void reset() noexcept {
    SPDLOG_TRACE("enter Scalar3::reset");

    v = Kokkos::mdspan<T, Kokkos::dextents<ui_t, 3>>();
    std::free(data);
    data = nullptr;

    SPDLOG_TRACE("exit Scalar3::reset");
  }
};

#endif // CORE_SCALAR_H
