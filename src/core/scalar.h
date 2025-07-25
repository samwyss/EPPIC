#ifndef CORE_SCALAR_H
#define CORE_SCALAR_H

#include <concepts>
#include <mdspan/mdspan.hpp>
#include <memory>

#include "coordinate.h"

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
  Scalar3(const Coord3<size_t> &dims, const T val) {
    const size_t n = dims.x * dims.y * dims.z;

    data_arr = std::make_unique<T[]>(n);
    data = Kokkos::mdspan(data_arr.get(), dims.x, dims.y, dims.z);

    for (size_t i = 0; i < n; ++i) {
      data_arr[i] = val;
    }
  }

  /// data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> data;

private:
  /// data container
  std::unique_ptr<T[]> data_arr;
};

#endif // CORE_SCALAR_H
