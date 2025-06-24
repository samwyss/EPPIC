#include "emengine.h"

FDTDGeometry::FDTDGeometry(const Config &config)
    : len({config.x_len, config.y_len, config.z_len}), ep_r(config.ep_r),
      mu_r(config.mu_r), ep(ep_r * VAC_PERMITTIVITY),
      mu(mu_r * VAC_PERMEABILITY), sigma(config.sigma) {

  // (m) minimum spatial step based on maximum frequency
  const double ds_min_wavelength =
      (VAC_SPEED_OF_LIGHT / (config.max_frequency * sqrt(ep_r * mu_r))) /
      static_cast<double>(config.num_vox_min_wavelength);

  // (m) minimum spatial step based on minimum feature size
  const double ds_min_feature_size =
      std::min({len.x, len.y, len.z}) /
      static_cast<double>(config.num_vox_min_feature);

  // (m) minimum required spatial step
  const double ds = std::min({ds_min_wavelength, ds_min_feature_size});

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(len.x / ds)),
        static_cast<size_t>(ceil(len.x / ds)),
        static_cast<size_t>(ceil(len.x / ds))};

  // (m) final spatial steps
  d = {len.x / static_cast<double>(nv.x), len.y / static_cast<double>(nv.y),
       len.z / static_cast<double>(nv.z)};

  // (m^-1) inverse spatial steps
  d_inv = {1.0 / d.x, 1.0 / d.y, 1.0 / d.z};

  // todo diagnostics
}

std::expected<FDTDGeometry, std::string>
FDTDGeometry::create(const Config &config) {
  try {
    return FDTDGeometry(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

FDTDEngine::FDTDEngine(const Config &config)
    : e(Vector3<double>({1, 1, 1}, 0.0)), h(Vector3<double>({1, 1, 1}, 0.0)) {}

std::expected<FDTDEngine, std::string>
FDTDEngine::create(const Config &config) {
  try {
    return FDTDEngine(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}
