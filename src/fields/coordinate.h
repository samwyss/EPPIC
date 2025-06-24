#ifndef FIELDS_COORDINATE_H
#define FIELDS_COORDINATE_H

template <typename T> struct Coord3 {
  T x;
  T y;
  T z;
};

template <typename T> struct Coord2 {
  T x;
  T y;
};

template <typename T> struct Coord1 {
  T x;
};

#endif // FIELDS_COORDINATE_H
