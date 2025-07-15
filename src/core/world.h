#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "config.h"
#include "fdtd_engine.h"
#include "hdf5_wrapper.h"
#include "type.h"

/*!
 * world object
 */
class World {
public:
  /*!
   * World static factory method
   * @param config configuration object
   * @return void
   */
  [[nodiscard]] static std::expected<World, std::string>
  create(Config &&config);

  /*!
   * advances internal state to an end time
   *
   * will do nothing in the event that end_t <= time
   * @param end_t (s) end time
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_to(fpp end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_by(fpp adv_t);

private:
  /*!
   * World constructor
   * @param config configuration object
   */
  explicit World(Config &&config);

  /// electromagnetic engine
  FDTDEngine engine;

  /// (s) elapsed time
  fpp time = 0.0;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio;

  /// output HDF5 file
  HDF5Obj h5;

  /// xdmf writer
  SimpleXdmf xdmf;
};

#endif // CORE_WORLD_H
