#ifndef CORE_EMENGINE_H
#define CORE_EMENGINE_H

#include <algorithm>
#include <expected>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

#include "config.h"
#include "coordinate.h"
#include "physical.h"
#include "vector.h"

/*!
 * FDTD geometry object
 * @tparam T floating point precision
 */
template <std::floating_point T> class FDTDGeometry {
public:
  /*!
   * FDTDGeometry static factory method
   * @param config configuration object
   * @return FDTDGeometry
   */
  static std::expected<FDTDGeometry, std::string> create(const Config &config);

  /// (m) size of bounding box in all directions
  const Coord3<T> len;

  /// relative diagonally isotropic permittivity of material inside bounding box
  const T ep_r;

  /// relative diagonally isotropic permeability of material inside bounding box
  const T mu_r;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const T ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const T mu;

  /// (S/m) diagonally isotropic conductivity of material inside bounding box
  const T sigma;

  /// (m) spatial increments in all directions
  Coord3<T> d;

  /// (m) inverse spatial increments in all directions
  Coord3<T> d_inv;

  /// number of voxels in all directions
  Coord3<size_t> nv;

private:
  explicit FDTDGeometry(const Config &config);
};

/*!
 * FDTD engine object
 * @tparam T floating point precision
 */
template <std::floating_point T> class FDTDEngine {
public:
  /*!
   * FDTDEngine static factory method
   * @param config configuration object
   * @return FDTDEngine
   */
  static std::expected<FDTDEngine, std::string> create(const Config &config);

  /*!
   * advances internal state to an end time
   *
   * will do nothing in the event that end_t <= time
   * @param end_t (s) end time
   * @return void
   */
  std::expected<void, std::string> advance_to(T end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  std::expected<void, std::string> advance_by(T adv_t);

private:
  /*!
   * FDTDEngine constructor
   * @param config configuration object
   */
  explicit FDTDEngine(const Config &config);

  /*!
   * calculates the number of steps required to model a given time span
   * @param time_span (s) time span to be modeled
   * @return number of steps required by CFL stability condition
   */
  [[nodiscard]] uint64_t calc_cfl_steps(T time_span) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  std::expected<void, std::string> step(T dt);

  /// FDTD geometry
  const FDTDGeometry<T> geom;

  /// (V/m) electric field vector
  Vector3<T> e;

  /// (A/m) magnetic field vector
  Vector3<T> h;

  /// (s) elapsed time
  T time = 0.0;
};

#endif // CORE_EMENGINE_H
