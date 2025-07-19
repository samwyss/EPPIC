#ifndef CORE_COORDINATE_H
#define CORE_COORDINATE_H

#include <type_traits>

/*!
 * arithmetic container with x, y, and z components
 * @tparam T arithmetic type
 */
template <typename T>
requires std::is_arithmetic_v<T>
struct Coord3 {
  T x;
  T y;
  T z;
};

/*!
 * arithmetic container with an x and y component
 * @tparam T arithmetic type
 */
template <typename T>
requires std::is_arithmetic_v<T>
struct Coord2 {
  T x;
  T y;
};

/*!
 * arithmetic container with an x component
 * @tparam T arithmetic type
 */
template <typename T>
requires std::is_arithmetic_v<T>
struct Coord1 {
  T x;
};

#endif // CORE_COORDINATE_H
