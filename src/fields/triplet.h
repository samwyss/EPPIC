#ifndef FIELDS_TRIPLET_H
#define FIELDS_TRIPLET_H

template <typename T> struct Triplet {
  /// x-component of triplet
  const T x;

  /// y-component of triplet
  const T y;

  /// z-component of triplet
  const T z;
};

#endif // FIELDS_TRIPLET_H
