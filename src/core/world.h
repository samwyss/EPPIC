#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <stdexcept>
#include <string>

#include "config.h"
#include "emengine.h"

/*!
 * world object
 */
class World {
public:
  /*!
   * World static factory method
   * @return void
   */
  static std::expected<World, std::string> create();

  /*!
   * advances internal state to an end time
   *
   * will do nothing in the event that end_t <= time
   * @param end_t (s) end time
   * @return void
   */
  std::expected<void, std::string> advance_to(double end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  std::expected<void, std::string> advance_by(double adv_t);

private:
  /*!
   * World constructor
   */
  World();

  /// model configuration
  const Config config;

  /// electromagnetic engine
  FDTDEngine emengine;

  /// (s) elapsed time
  double time = 0.0;
};

#endif // CORE_WORLD_H
