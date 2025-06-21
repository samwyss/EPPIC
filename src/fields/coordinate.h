#ifndef FIELDS_COORDINATE_H
#define FIELDS_COORDINATE_H

template <typename T> struct Triplet {
  /// x-component
  const T x;

  /// y-component
  const T y;

  /// z-component
  const T z;
};

template <typename T> struct Pair {
  /// x-component
  const T x;

  /// y-component
  const T y;
};

#endif // FIELDS_COORDINATE_H
