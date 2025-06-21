#include "scalar.h"

template <typename T>
Scalar3<T>::Scalar3(const size_t num_x, const size_t num_y, const size_t num_z, const T value) :
    num_cells(Triplet<size_t>{num_x, num_y, num_z}), row_cells(num_z), plane_cells(num_y * num_z),
    vol_cells(num_x * num_y * num_z)
{
  // create data array
  data = new T[vol_cells];

  // initialize data array with value with overloaded assignment operator
  (*this) = value;
}

template <typename T>
Scalar3<T>::~Scalar3()
{
  delete[] data;
}

template <typename T>
T& Scalar3<T>::operator()(const size_t i, const size_t j, const size_t k)
{
  return data[k + row_cells * j + plane_cells * i];
}

template <typename T>
const T& Scalar3<T>::operator()(const size_t i, const size_t j, const size_t k) const
{
  return data[k + row_cells * j + plane_cells * i];
}

template <typename T>
Scalar3<T>& Scalar3<T>::operator=(const T& rhs)
{
  // assign data array with rhs
  for (size_t i = 0; i < vol_cells; ++i)
  {
    data[i] = rhs;
  }

  return *this;
}


// explicit template instantiations
template class Scalar3<double>;
template class Scalar3<uint64_t>;
