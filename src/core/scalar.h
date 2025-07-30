#ifndef CORE_SCALAR_H
#define CORE_SCALAR_H

#include <concepts>
#include <cstdlib>
#include <mdspan/mdspan.hpp>

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

    data_arr = static_cast<T *>(std::aligned_alloc(64, sizeof(T) * n));
    data = Kokkos::mdspan(data_arr, dims.x, dims.y, dims.z);

    for (size_t i = 0; i < n; ++i) {
      data_arr[i] = val;
    }
  }

  /*!
   * Scalar destructor
   */
  ~Scalar3() { std::free(data_arr); }

  /// data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> data;

private:
  /// data container
  T *data_arr;
};

#endif // CORE_SCALAR_H
