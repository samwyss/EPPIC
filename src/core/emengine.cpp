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
    : geom(FDTDGeometry::create(config).value()),
      e(Vector3<double>(geom.nv, 0.0)), h(geom.nv, 0.0) {}

std::expected<FDTDEngine, std::string>
FDTDEngine::create(const Config &config) {
  try {
    return FDTDEngine(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> FDTDEngine::advance_to(const double end_t) {
  if (end_t > time) {
    // (s) time difference between current state and end time
    const double adv_t = end_t - time;

    advance_by(adv_t).value();
  }
  return {};
}

std::expected<void, std::string> FDTDEngine::advance_by(const double adv_t) {
  // number of steps required by CFL condition
  const size_t steps = calc_cfl_steps(adv_t);

  // (s) time step
  const double dt = adv_t / static_cast<double>(steps);

  // preprocess loop constants
  const double ea = 1.0 / (geom.ep / dt + geom.sigma / 2.0);
  const double eb = geom.ep / dt - geom.sigma / 2.0;
  const double hxa = dt * geom.d_inv.x / geom.mu;
  const double hya = dt * geom.d_inv.y / geom.mu;
  const double hza = dt * geom.d_inv.z / geom.mu;

  // todo pre loop diagnostics
  spdlog::debug("steps: {}", steps);
  spdlog::debug("num vox: {}", e.x.size());

  // main time loop
  for (uint64_t i = 0; i < steps; ++i) {
    step(dt).value();
    // spdlog::debug("step: " + std::to_string(i));
  }

  return {};
}

uint64_t FDTDEngine::calc_cfl_steps(const double time_span) const {

  // todo use speed of light calculated from relative properties instead of
  // todo vacuum

  const double dt = 1.0 / (VAC_SPEED_OF_LIGHT *
                           sqrt(pow(geom.d_inv.x, 2) + pow(geom.d_inv.y, 2) +
                                pow(geom.d_inv.z, 2)));

  return static_cast<uint64_t>(time_span / dt);
}

std::expected<void, std::string> FDTDEngine::step(const double dt) {
  return {};
}