#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "config.h"
#include "fdtd_engine.h"

/*!
 * world object
 * @tparam T floating point precision
 */
template <std::floating_point T> class World {
public:
  /*!
   * World static factory method
   * @param config configuration object
   * @return void
   */
  [[nodiscard]] static std::expected<World, std::string>
  create(const Config<T> &config);

  /*!
   * advances internal state to an end time
   *
   * will do nothing in the event that end_t <= time
   * @param end_t (s) end time
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_to(T end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_by(T adv_t);

private:
  /*!
   * World constructor
   * @param config configuration object
   */
  explicit World(const Config<T> &config);

  /// electromagnetic engine
  FDTDEngine<T> engine;

  /// (s) elapsed time
  T time = 0.0;
};

#endif // CORE_WORLD_H
