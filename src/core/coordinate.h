#ifndef CORE_COORDINATE_H
#define CORE_COORDINATE_H

#include <concepts>
#include <type_traits>

template <typename T>
concept numeric = std::is_arithmetic_v<T>;

/*!
 * arithmetic container with x, y, and z components
 * @tparam T numeric type
 */
template <numeric T> struct Coord3 {
  T x;
  T y;
  T z;
};

/*!
 * arithmetic container with an x and y component
 * @tparam T numeric type
 */
template <numeric T> struct Coord2 {
  T x;
  T y;
};

/*!
 * arithmetic container with an x component
 * @tparam T numeric type
 */
template <numeric T> struct Coord1 {
  T x;
};

#endif // CORE_COORDINATE_H
