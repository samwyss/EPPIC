#ifndef FIELDS_VECTOR_H
#define FIELDS_VECTOR_H

#include "scalar.h"

template <typename T> class Vector3 {
public:
  Scalar3<T> x;

  Scalar3<T> y;

  Scalar3<T> z;
};

#endif // FIELDS_VECTOR_H
