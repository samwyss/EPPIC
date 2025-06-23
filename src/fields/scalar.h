#ifndef FIELDS_SCALAR_H
#define FIELDS_SCALAR_H

#include <mdspan/mdspan.hpp>
#include <memory>

#include "coordinate.h"

template <typename T> struct Scalar3 {
  /*!
   * Scalar3 constructor
   * @param dims vector field dimensions
   * @param val initial field value
   */
  Scalar3(const Coord3<size_t> &dims, const T val) {
    const size_t nelems = dims.x * dims.y * dims.z;

    data_arr = std::make_unique<T[]>(nelems);

    data = Kokkos::mdspan(data_arr.get(), dims);

    for (auto elem : data) {
      elem = val;
    }
  }

  /*!
   * Scalar3 destructor
   */
  ~Scalar3() = default;

  /// data view
  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> data;

private:
  // data container
  std::unique_ptr<T[]> data_arr;
};

#endif // FIELDS_SCALAR_H
