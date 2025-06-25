#include "emengine.h"

template <std::floating_point T>
FDTDGeometry<T>::FDTDGeometry(const Config &config)
    : len({config.x_len, config.y_len, config.z_len}), ep_r(config.ep_r),
      mu_r(config.mu_r), ep(ep_r * VAC_PERMITTIVITY<T>),
      mu(mu_r * VAC_PERMEABILITY<T>), sigma(config.sigma) {

  // (m) minimum spatial step based on maximum frequency
  const T ds_min_wavelength =
      (VAC_SPEED_OF_LIGHT<T> / (config.max_frequency * sqrt(ep_r * mu_r))) /
      static_cast<T>(config.num_vox_min_wavelength);

  // (m) minimum spatial step based on minimum feature size
  const T ds_min_feature_size = std::min({len.x, len.y, len.z}) /
                                static_cast<T>(config.num_vox_min_feature);

  // (m) minimum required spatial step
  const T ds = std::min({ds_min_wavelength, ds_min_feature_size});

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(static_cast<double>(len.x) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.y) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.z) / ds))};

  // (m) final spatial steps
  d = {len.x / static_cast<T>(nv.x), len.y / static_cast<T>(nv.y),
       len.z / static_cast<T>(nv.z)};

  // (m^-1) inverse spatial steps
  d_inv = {1.0 / d.x, 1.0 / d.y, 1.0 / d.z};

  // todo diagnostics
}

template <std::floating_point T>
std::expected<FDTDGeometry<T>, std::string>
FDTDGeometry<T>::create(const Config &config) {
  try {
    return FDTDGeometry(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
FDTDEngine<T>::FDTDEngine(const Config &config)
    : geom(FDTDGeometry<T>::create(config).value()), e(geom.nv, 0.0),
      h(geom.nv, 0.0) {}

template <std::floating_point T>
std::expected<FDTDEngine<T>, std::string>
FDTDEngine<T>::create(const Config &config) {
  try {
    return FDTDEngine(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
std::expected<void, std::string> FDTDEngine<T>::advance_to(const T end_t) {
  if (end_t > time) {
    // (s) time difference between current state and end time
    const T adv_t = end_t - time;

    advance_by(adv_t).value();
  }
  return {};
}

template <std::floating_point T>
std::expected<void, std::string> FDTDEngine<T>::advance_by(const T adv_t) {
  // number of steps required by CFL condition
  const size_t steps = calc_cfl_steps(adv_t);

  // (s) time step
  const T dt = adv_t / static_cast<T>(steps);

  // preprocess loop constants
  const T ea = 1.0 / (geom.ep / dt + geom.sigma / 2.0);
  const T eb = geom.ep / dt - geom.sigma / 2.0;
  const T hxa = dt * geom.d_inv.x / geom.mu;
  const T hya = dt * geom.d_inv.y / geom.mu;
  const T hza = dt * geom.d_inv.z / geom.mu;

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

template <std::floating_point T>
uint64_t FDTDEngine<T>::calc_cfl_steps(const T time_span) const {

  // todo use speed of light calculated from relative properties instead of
  // todo vacuum

  const T dt = 1.0 / (VAC_SPEED_OF_LIGHT<T> *
                      sqrt(pow(geom.d_inv.x, 2) + pow(geom.d_inv.y, 2) +
                           pow(geom.d_inv.z, 2)));

  return static_cast<uint64_t>(ceil(time_span / dt));
}

template <std::floating_point T>
std::expected<void, std::string> FDTDEngine<T>::step(const T dt) {
  return {};
}

// explicit template instantiation
template class FDTDGeometry<double>;
template class FDTDGeometry<float>;
template class FDTDEngine<double>;
template class FDTDEngine<float>;