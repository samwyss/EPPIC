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
#include "type.h"
#include "vector.h"

/*!
 * FDTD engine object
 */
class FDTDEngine {
public:
  /*!
   * FDTDEngine static factory method
   * @param config configuration object
   * @return FDTDEngine
   */
  [[nodiscard]] static std::expected<FDTDEngine, std::string>
  create(const Config &config);

  /*!
   * calculates the number of steps required to advance engine state by some
   * time period
   * @param adv_t (s) time period to advance by
   * @return uint64_t
   */
  [[nodiscard]] uint64_t calc_num_steps(fpp adv_t) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  void step(fpp dt);

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
  explicit FDTDEngine(const Config &config);

  /*!
   * calculates the number of steps required to model a given time span
   * @param time_span (s) time span to be modeled
   * @return number of steps required by CFL stability condition
   */
  [[nodiscard]] uint64_t calc_cfl_steps(fpp time_span) const;

  /*!
   * advances internal electric field state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_e(fpp ea, fpp eb) const;

  /*!
   * advances internal magnetic field state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_h(fpp hxa, fpp hya, fpp hza) const;

  /*!
   * advances internal electric field x-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ex(fpp ea, fpp eb) const;
  /*!
   * advances internal electric field y-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ey(fpp ea, fpp eb) const;

  /*!
   * advances internal electric field z-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ez(fpp ea, fpp eb) const;

  /*!
   * advances internal magnetic field x-component state by one time step
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hx(fpp hya, fpp hza) const;

  /*!
   * advances internal magnetic field y-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hy(fpp hxa, fpp hza) const;

  /*!
   * advances internal magnetic field z-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   */
  void update_hz(fpp hxa, fpp hya) const;

  /// (m) size of bounding box in all directions
  const Coord3<fpp> len;

  /// relative diagonally isotropic permittivity of material inside bounding box
  const fpp ep_r;

  /// relative diagonally isotropic permeability of material inside bounding box
  const fpp mu_r;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const fpp ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const fpp mu;

  /// (S/m) diagonally isotropic conductivity of material inside bounding box
  const fpp sigma;

  /// number of voxels in all directions
  Coord3<size_t> nv{};

  /// (m) spatial increments in all directions
  Coord3<fpp> d;

  /// (m) inverse spatial increments in all directions
  Coord3<fpp> d_inv;

  /// (V/m) electric field vector
  Vector3<fpp> e;

  /// (A/m) magnetic field vector
  Vector3<fpp> h;

  /// (s) elapsed time
  fpp time = 0.0;
};

#endif // CORE_FDTD_ENGINE_H
