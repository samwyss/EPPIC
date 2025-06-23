#ifndef FIELDS_VECTOR_H
#define FIELDS_VECTOR_H

#include <memory>

#include "mdspan/mdspan.hpp"

template <typename T> struct Vector3 {

  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> x;

  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> y;

  Kokkos::mdspan<double, Kokkos::dextents<size_t, 3>> z;

private:
  std::unique_ptr<T[]> x_data;

  std::unique_ptr<T[]> y_data;

  std::unique_ptr<T[]> z_data;
};

#endif // FIELDS_VECTOR_H
