#ifndef CORE_FDTD_ENGINE_H
#define CORE_FDTD_ENGINE_H

#include <algorithm>
#include <expected>
#include <hdf5/openmpi/hdf5.h>
#include <simple_xdmf.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <stdexcept>
#include <string>

#include "config.h"
#include "coordinate.h"
#include "numeric.h"
#include "physical.h"
#include "vector.h"

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
  [[nodiscard]] static std::expected<FDTDEngine, std::string>
  create(const Config<T> &config);

  /*!
   * calculates the number of steps required to advance engine state by some
   * time period
   * @param adv_t (s) time period to advance by
   * @return uint64_t
   */
  [[nodiscard]] uint64_t calc_num_steps(T adv_t) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  void step(T dt);

  /*!
   * returns the number of total voxels per field
   * @return uint64_t
   */
  [[nodiscard]] uint64_t get_field_num_vox() const;

private:
  /*!
   * FDTDEngine constructor
   * @param config configuration object
   */
  explicit FDTDEngine(const Config<T> &config);

  /*!
   * calculates the number of steps required to model a given time span
   * @param time_span (s) time span to be modeled
   * @return number of steps required by CFL stability condition
   */
  [[nodiscard]] uint64_t calc_cfl_steps(T time_span) const;

  /*!
   * advances internal electric field state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_e(T ea, T eb);

  /*!
   * advances internal magnetic field state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_h(T hxa, T hya, T hza);

  /*!
   * advances internal electric field x-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ex(T ea, T eb);
  /*!
   * advances internal electric field y-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ey(T ea, T eb);

  /*!
   * advances internal electric field z-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ez(T ea, T eb);

  /*!
   * advances internal magnetic field x-component state by one time step
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hx(T hya, T hza);

  /*!
   * advances internal magnetic field y-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hy(T hxa, T hza);

  /*!
   * advances internal magnetic field z-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   */
  void update_hz(T hxa, T hya);

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

  /// number of voxels in all directions
  Coord3<size_t> nv{};

  /// (m) spatial increments in all directions
  Coord3<T> d;

  /// (m) inverse spatial increments in all directions
  Coord3<T> d_inv;

  /// (V/m) electric field vector
  Vector3<T> e;

  /// (A/m) magnetic field vector
  Vector3<T> h;

  /// (s) elapsed time
  T time = 0.0;
};

#endif // CORE_FDTD_ENGINE_H
