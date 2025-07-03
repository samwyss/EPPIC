#ifndef FIELDS_VECTOR_H
#define FIELDS_VECTOR_H

#include <mdspan/mdspan.hpp>
#include <memory>

#include "coordinate.h"

/*!
 * 3D vector field
 * @tparam T arithmetic type
 */
template <typename T>
  requires std::is_arithmetic_v<T>
class Vector3 {
public:
  /*!
   * Vector3 default constructor
   */
  Vector3() = default;

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

    x = Kokkos::mdspan(x_data.get(), dims.x, dims.y, dims.z);
    y = Kokkos::mdspan(y_data.get(), dims.x, dims.y, dims.z);
    z = Kokkos::mdspan(z_data.get(), dims.x, dims.y, dims.z);

    for (size_t i = 0; i < nelems; ++i) {
      x_data[i] = val;
      y_data[i] = val;
      z_data[i] = val;
    }
  }

  /// x-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> x;

  /// y-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> y;

  /// z-component data view
  Kokkos::mdspan<T, Kokkos::dextents<size_t, 3>> z;

private:
  /// x-component data container
  std::unique_ptr<T[]> x_data;

  /// y-component data container
  std::unique_ptr<T[]> y_data;

  /// z-component data container
  std::unique_ptr<T[]> z_data;
};

#endif // FIELDS_VECTOR_H
