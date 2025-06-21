#ifndef FIELDS_SCALAR_H
#define FIELDS_SCALAR_H

#include <cstddef>
#include <mdspan>
#include <memory>

#include "coordinate.h"

template <typename T> struct Scalar3 {
  /*!
   * Scalar3 constructor
   * @param nx number of elements in the x-direction
   * @param ny number of elements in the y-direction
   * @param nz number of elements in the z-direction
   * @param val initial value
   */
  Scalar3(const size_t nx, const size_t ny, const size_t nz, T val)
      : cell_counts(Triplet{nx, ny, nz}), row_off(nz), plane_off(ny * nz),
        num_cells(nx * ny * nz) {
    // create data container
    data = std::make_unique<T[]>(num_cells);

    // initialize data
    *this = val;
  }

  /*!
   * Scalar3 destructor
   */
  ~Scalar3() = default;

  /*!
   * Scalar3 overloaded () operator for read-write access
   * @param i index into x-direction elements
   * @param j index into y-direction elements
   * @param k index into z-direction elements
   * @return mutable element at desired index
   */
  T &operator()(const size_t i, const size_t j, const size_t k) {
    return data.get()[k + row_off * j + plane_off * i];
  }

  /*!
   * Scalar3 overloaded () operator for read access
   * @param i index into x-direction elements
   * @param j index into y-direction elements
   * @param k index into z-direction elements
   * @return element at desired index
   */
  T operator()(const size_t i, const size_t j, const size_t k) const {
    return data.get()[k + row_off * j + plane_off * i];
  };

private:
  /*!
   * Scalar3 overloaded assignment operator to initialize field
   * @param val initial value
   * @return reference to existing Scalar3
   */
  Scalar3 &operator=(const T &val) {
    for (size_t i = 0; i < num_cells; ++i) {
      data.get()[i] = val;
    }

    return *this;
  }

  /// 1D array in row-major format containing field data
  std::unique_ptr<T[]> data;

  /// number of cells in each direction
  const Triplet<size_t> cell_counts;

  /// row offset into data
  const size_t row_off;

  /// column offset into data
  const size_t plane_off;

  /// number of cells in bounding box volume
  const size_t num_cells;
};

#endif // FIELDS_SCALAR_H
