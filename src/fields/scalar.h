#ifndef FIELDS_SCALAR_H
#define FIELDS_SCALAR_H

#include <mdspan/mdspan.hpp>
#include <memory>

#include "coordinate.h"

template <typename T> class Scalar3 {
public:
  /*!
   * Scalar3 constructor
   * @param dims field dimensions
   * @param val initial field value
   */
  Scalar3(const Coord3<size_t> &dims, const T val) {
    const size_t nelems = dims.x * dims.y * dims.z;

    data_arr = std::make_unique<T[]>(nelems);

    data = Kokkos::mdspan(data_arr.get(), dims.x, dims.y, dims.z);

    for (size_t i = 0; i < nelems; ++i) {
      data_arr[i] = val;
    }
  }

  /*!
   * Scalar3 destructor
   */
  ~Scalar3() = default;

  /// data view
  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> data;

private:
  /// data container
  std::unique_ptr<T[]> data_arr;
};

#endif // FIELDS_SCALAR_H
