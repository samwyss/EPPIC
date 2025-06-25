#ifndef CORE_EMENGINE_H
#define CORE_EMENGINE_H

#include <algorithm>
#include <expected>
#include <stdexcept>
#include <string>

#include "config.h"
#include "coordinate.h"
#include "physical.h"
#include "vector.h"

class FDTDGeometry {
public:
  /*!
   * FDTDGeometry static factory method
   * @param config configuration object
   * @return FDTDGeometry
   */
  static std::expected<FDTDGeometry, std::string> create(const Config &config);

  /// (m) size of bounding box in all directions
  const Coord3<double> len;

  /// relative diagonally isotropic permittivity of material inside bounding box
  const double ep_r;

  /// relative diagonally isotropic permeability of material inside bounding box
  const double mu_r;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const double ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const double mu;

  /// (S/m) diagonally isotropic conductivity of material inside bounding box
  const double sigma;

  /// (m) spatial increments in all directions
  Coord3<double> d;

  /// (m) inverse spatial increments in all directions
  Coord3<double> d_inv;

  /// number of voxels in all directions
  Coord3<size_t> nv;

private:
  explicit FDTDGeometry(const Config &config);
};

class FDTDEngine {
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
  std::expected<void, std::string> advance_to(double end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  std::expected<void, std::string> advance_by(double adv_t);

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
  [[nodiscard]] uint64_t calc_cfl_steps(double time_span) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  std::expected<void, std::string> step(double dt);

  /// FDTD geometry
  const FDTDGeometry geom;

  /// (V/m) electric field vector
  Vector3<double> e;

  /// (A/m) magnetic field vector
  Vector3<double> h;

  /// (s) elapsed time
  double time = 0.0;
};

#endif // CORE_EMENGINE_H
