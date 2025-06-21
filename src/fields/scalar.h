#ifndef FIELDS_SCALAR_H
#define FIELDS_SCALAR_H

#include <cstddef>

#include "triplet.h"

template <typename T> class Scalar3 {
public:
  /*!
   * Scalar3 constructor
   * @param num_x number of cells in the x-direction
   * @param num_y number of cells in the y-direction
   * @param num_z number of cells in the z-direction
   * @param value initial value to instantiate all field values to
   */
  Scalar3(const size_t num_x, const size_t num_y, const size_t num_z, T value)
      : cell_counts(Triplet<size_t>{num_x, num_y, num_z}), row_off(num_z),
        plane_off(num_y * num_z), num_cells(num_x * num_y * num_z) {
    // create data array
    data = new T[num_cells];

    // initialize data array with value with overloaded assignment operator
    (*this) = value;
  }

  /*!
   * Scalar3 destructor
   */
  ~Scalar3() { delete[] data; }

  /*!
   * Scalar3 overloaded () operator for read-write access
   * @param i index into x-direction elements
   * @param j index into y-direction elements
   * @param k index into z-direction elements
   * @return pointer to element at desired index
   */
  T &operator()(const size_t i, const size_t j, const size_t k) {
    return data[k + row_off * j + plane_off * i];
  }

  /*!
   * Scalar3 overloaded () operator for read access
   * @param i index into x-direction elements
   * @param j index into y-direction elements
   * @param k index into z-direction elements
   * @return pointer to element at desired index
   */
  T operator()(const size_t i, const size_t j, const size_t k) const {
    return data[k + row_off * j + plane_off * i];
  };

  /*!
   * Scalar3 overloaded assignment operator to spatially constant scalar value
   * @param rhs spatially constant scalar value
   * @return reference to existing Scalar3
   */
  Scalar3 &operator=(const T &rhs) {
    // assign data array with rhs
    for (size_t i = 0; i < num_cells; ++i) {
      data[i] = rhs;
    }

    return *this;
  }

private:
  /// 1D array in row-major format containing field data
  T *data;

  /// number of cells in each direction
  const Triplet<size_t> cell_counts;

  /// row offset into data
  const size_t row_off;

  /// collumn offset into data
  const size_t plane_off;

  /// number of cells in bounding box volume
  const size_t num_cells;
};

#endif // FIELDS_SCALAR_H
