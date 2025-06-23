#ifndef FIELDS_VECTOR_H
#define FIELDS_VECTOR_H

#include <mdspan/mdspan.hpp>
#include <memory>

#include "coordinate.h"

template <typename T> struct Vector3 {
  /*!
   * Vector3 constructor
   * @param dims field dimensions
   * @param val initial field value
   */
  Vector3(const Coord3<size_t> &dims, const T val) {
    const size_t nelems = dims.x * dims.y * dims.z;

    x_data = std::make_unique<T[]>(nelems);
    y_data = std::make_unique<T[]>(nelems);
    z_data = std::make_unique<T[]>(nelems);

    x = Kokkos::mdspan(x_data.get(), dims);
    y = Kokkos::mdspan(y_data.get(), dims);
    z = Kokkos::mdspan(z_data.get(), dims);

    for (size_t i = 0; i < nelems; i++) {
      x_data[i] = val;
      y_data[i] = val;
      z_data[i] = val;
    }
  }

  /*!
   * Vector3 destructor
   */
  ~Vector3() = default;

  /// x-component data view
  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> x;

  /// y-component data view
  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> y;

  /// z-component data view
  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> z;

private:
  /// x-component data container
  std::unique_ptr<T[]> x_data;

  /// y-component data container
  std::unique_ptr<T[]> y_data;

  /// z-component data container
  std::unique_ptr<T[]> z_data;
};

#endif // FIELDS_VECTOR_H
