#ifndef CORE_EMENGINE_H
#define CORE_EMENGINE_H

#include <algorithm>
#include <expected>
#include <stdexcept>
#include <string>

#include "config.h"
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

  /// (m) size of bounding box in the x-direction
  const double x_len;

  /// (m) size of bounding box in the y-direction
  const double y_len;

  /// (m) size of bounding box in the z-direction
  const double z_len;

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

  /// (m) spatial increment in x-direction
  double dx;

  /// (m) spatial increment in y-direction
  double dy;

  /// (m) spatial increment in z-direction
  double dz;

  /// (m) inverse spatial increment in x-direction
  double dx_inv;

  /// (m) inverse spatial increment in y-direction
  double dy_inv;

  /// (m) inverse spatial increment in z-direction
  double dz_inv;

  /// number of voxels in x-direction
  size_t nvx;

  /// number of voxels in y-direction
  size_t nvy;

  /// number of voxels in z-direction
  size_t nvz;

private:
  FDTDGeometry(const Config &config);
};

class FDTDEngine {
public:
  /*!
   * FDTDEngine static factory method
   * @param config configuration object
   * @return FDTDEngine
   */
  static std::expected<FDTDEngine, std::string> create(const Config &config);

private:
  /*!
   * FDTDEngine constructor
   * @param config configuration object
   */
  explicit FDTDEngine(const Config &config);

  /// (V/m) electric field vector
  Vector3<double> e;

  /// (A/m) magnetic field vector
  Vector3<double> h;

  /// (s) elapsed time
  double time = 0.0;
};

#endif // CORE_EMENGINE_H
