#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "config.h"
#include "fdtd_engine.h"
#include "type.h"

/*!
 * RAII HDF5 object wrapper
 * @tparam CloseFunc function that releases HDF5 object
 */
template <typename CloseFunc> class HDF5Mgr {
public:
  /*!
   * HDF5 object wrapper constructor
   * @param handle HDF5 object handle
   * @param release HDF5 object release function
   */
  HDF5Mgr(const hid_t handle, const CloseFunc release)
      : handle(handle), close(release) {}

  /*!
   * HDF5 object wrapper destructor
   */
  ~HDF5Mgr() noexcept {
    if (handle >= 0) {
      close(handle);
    }
  }

  /*!
   * deleted HDF5 object wrapper copy constructor
   */
  HDF5Mgr(const HDF5Mgr &) = delete;

  /*!
   * deleted HDF5 object wrapper copy assignment operator
   */
  HDF5Mgr &operator=(const HDF5Mgr &) = delete;

  /*!
   * HDF5 object wrapper move constructor
   * @param other other HDF5 object wrapper
   */
  HDF5Mgr(HDF5Mgr &&other) noexcept
      : handle(std::exchange(other.handle, -1)),
        close(std::move(other.close)) {};

  /*!
   * HDF5 object wrapper move assignment operator
   * @param other other HDF5 object wrapper
   * @return HDF5 obbject wrapper
   */
  HDF5Mgr &operator=(HDF5Mgr &&other) noexcept {
    if (this != &other) {
      if (handle >= 0) {
        close(handle);
      }

      handle = std::exchange(other.handle, -1);

      close = std::move(other.close);
    }
    return *this;
  }

  /*!
   * HDF5 object handle getter
   * @return HDF5 object handle
   */
  hid_t get() const noexcept { return handle; }

private:
  /// HDF5 object handle
  hid_t handle;

  /// HDF5 object release function
  const CloseFunc close;
};

/// type alias for HDF5Mgr
using HDF5Obj = HDF5Mgr<herr_t (*)(hid_t)>;

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
  create(const Config &config);

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
  explicit World(const Config &config);

  /// electromagnetic engine
  FDTDEngine engine;

  /// (s) elapsed time
  fpp time = 0.0;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio;

  /// output HDF5 file
  HDF5Obj file;
};

#endif // CORE_WORLD_H
