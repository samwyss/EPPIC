#ifndef FIELDS_COORDINATE_H
#define FIELDS_COORDINATE_H

template <typename T> struct Coord3 {
  const T x;
  const T y;
  const T z;
};

template <typename T> struct Coord2 {
  const T x;
  const T y;
};

template <typename T> struct Coord1 {
  const T x;
};

#endif // FIELDS_COORDINATE_H
