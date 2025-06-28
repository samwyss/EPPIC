#include "emengine.h"

#include "numeric.h"

template <std::floating_point T>
FDTDGeometry<T>::FDTDGeometry(const Config<T> &config)
    : len({config.x_len, config.y_len, config.z_len}), ep_r(config.ep_r),
      mu_r(config.mu_r), ep(ep_r * VAC_PERMITTIVITY<T>),
      mu(mu_r * VAC_PERMEABILITY<T>), sigma(config.sigma) {

  SPDLOG_TRACE("enter FDTDGeometry<T>::FDTDGeometry");
  SPDLOG_DEBUG("FDTD bounding box (m): {:.3e} x {:.3e} x {:.3e}", len.x, len.y,
               len.z);

  // (m) maximum spatial step based on maximum frequency
  const T ds_min_wavelength =
      VAC_SPEED_OF_LIGHT<T> /
      (sqrt(ep_r * mu_r) * static_cast<T>(config.num_vox_min_wavelength) *
       config.max_frequency);
  SPDLOG_DEBUG(
      "FDTD maximum spatial step based on maximum frequency (m): {:.3e}",
      ds_min_wavelength);

  // (m) maximum spatial step based on minimum feature size
  const T ds_min_feature_size = std::min({len.x, len.y, len.z}) /
                                static_cast<T>(config.num_vox_min_feature);
  SPDLOG_DEBUG("FDTD maximum spatial step based on feature size (m): {:.3e}",
               ds_min_feature_size);

  // (m) maximum required spatial step
  const T ds = std::min({ds_min_wavelength, ds_min_feature_size});
  SPDLOG_DEBUG("FDTD maximum spatial step (m): {:.3e}", ds);

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(static_cast<double>(len.x) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.y) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.z) / ds))};
  SPDLOG_DEBUG("FDTD voxels: {} x {} x {}", nv.x, nv.y, nv.z);

  // (m) final spatial steps
  d = {len.x / static_cast<T>(nv.x), len.y / static_cast<T>(nv.y),
       len.z / static_cast<T>(nv.z)};
  SPDLOG_DEBUG("FDTD voxel size (m): {:.3e} x {:.3e} x {:.3e}", d.x, d.y, d.z);

  // (m^-1) inverse spatial steps
  d_inv = {static_cast<T>(1.0) / d.x, static_cast<T>(1.0) / d.y,
           static_cast<T>(1.0) / d.z};
  SPDLOG_DEBUG("FDTD inverse voxel size (m) , {:.3e} x {:.3e} x {:.3e}",
               d_inv.x, d_inv.y, d_inv.z);

  SPDLOG_TRACE("exit FDTDGeometry<T>::FDTDGeometry");
}

template <std::floating_point T>
std::expected<FDTDGeometry<T>, std::string>
FDTDGeometry<T>::create(const Config<T> &config) {
  SPDLOG_TRACE("enter FDTDGeometry<T>::create");
  try {
    const FDTDGeometry geom(config);
    SPDLOG_TRACE("exit FDTDGeometry<T>::create with success");
    return geom;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit FDTDGeometry<T>::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
FDTDEngine<T>::FDTDEngine(const Config<T> &config)
    : geom(FDTDGeometry<T>::create(config).value()), e(geom.nv, 0.0),
      h(geom.nv, 0.0) {
  SPDLOG_TRACE("enter FDTDEngine<T>::FDTDEngine");
  SPDLOG_TRACE("exit FDTDEngine<T>::FDTDEngine");
}

template <std::floating_point T>
std::expected<FDTDEngine<T>, std::string>
FDTDEngine<T>::create(const Config<T> &config) {
  SPDLOG_TRACE("enter FDTDEngine<T>::create");
  try {
    // todo why not const like FDTD geometry?
    FDTDEngine engine(config);
    SPDLOG_TRACE("exit FDTDEngine<T>::create with success");
    return engine;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit FDTDEngine<T>::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
std::expected<void, std::string> FDTDEngine<T>::advance_to(const T end_t) {
  SPDLOG_TRACE("enter FDTDEngine<T>::advance_to");
  SPDLOG_DEBUG("FDTD current time is {:.3e} (s)", time);
  SPDLOG_DEBUG("FDTD advance time to {:.3e} (s)", end_t);
  if (end_t > time) {
    // (s) time difference between current state and end time
    const T adv_t = end_t - time;

    if (const auto adv_by_result = advance_by(adv_t);
        !adv_by_result.has_value()) {
      SPDLOG_CRITICAL("FDTD advance time by advance_by returned with error: {}",
                      adv_by_result.error());
      return std::unexpected(adv_by_result.error());
    }

  } else {
    SPDLOG_WARN(
        "end time of {:.3e} (s) is not greater than current time of {:.3e} (s)",
        end_t, time);
  }

  SPDLOG_TRACE("exit FDTDEngine<T>::advance_to");
  return {};
}

template <std::floating_point T>
std::expected<void, std::string> FDTDEngine<T>::advance_by(const T adv_t) {
  SPDLOG_TRACE("enter FDTDEngine<T>::advance_by");
  SPDLOG_DEBUG("FDTD advance time by {:.3e} (s)", adv_t);

  // (s) initial time
  // only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  [[maybe_unused]] const auto init_time = time;

  // number of steps required by CFL condition
  const size_t steps = calc_cfl_steps(adv_t);

  // (s) time step
  const T dt = adv_t / static_cast<T>(steps);
  SPDLOG_DEBUG("FDTD timestep: {:.3e} (s)", dt);

  // preprocess loop constants
  const T ea = 1.0 / (geom.ep / dt + geom.sigma / 2.0);
  const T eb = geom.ep / dt - geom.sigma / 2.0;
  const T hxa = dt * geom.d_inv.x / geom.mu;
  const T hya = dt * geom.d_inv.y / geom.mu;
  const T hza = dt * geom.d_inv.z / geom.mu;

  // main time loop
  SPDLOG_DEBUG("FDTD enter main time loop");
  try {
    for (uint64_t i = 0; i < steps; ++i) {

      // advance by one step
      step(dt, ea, eb, hxa, hya, hza);

      SPDLOG_TRACE("step: {}/{} elapsed time: {:.5e}/{:.5e} (s)", i + 1, steps,
                   time, init_time + adv_t);
    }
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("FDTD main time loop returned with error: {}", err.what());
    return std::unexpected(err.what());
  }
  SPDLOG_DEBUG("FDTD exit main time loop with success");

  SPDLOG_TRACE("exit FDTDEngine<T>::advance_by");
  return {};
}

template <std::floating_point T>
uint64_t FDTDEngine<T>::calc_cfl_steps(const T time_span) const {
  SPDLOG_TRACE("enter FDTDEngine<T>::calc_cfl_steps");

  const T maximum_dt =
      1.0 / (VAC_SPEED_OF_LIGHT<T> / sqrt(geom.ep_r * geom.mu_r) *
             sqrt(pow(geom.d_inv.x, 2) + pow(geom.d_inv.y, 2) +
                  pow(geom.d_inv.z, 2)));
  SPDLOG_DEBUG(
      "FDTD maximum possible timesteps to satisfy CFL condition: {:.3e} (s)",
      maximum_dt);

  const T num_steps = static_cast<uint64_t>(ceil(time_span / maximum_dt));
  SPDLOG_DEBUG("FDTD steps required to satisfy CFL condition: {}", num_steps);

  SPDLOG_TRACE("exit FDTDEngine<T>::calc_cfl_steps");
  return num_steps;
}

template <std::floating_point T>
void FDTDEngine<T>::step(const T dt, const T ea, const T eb, const T hxa,
                         const T hya, const T hza) {
  // half timestep update before updating magnetic fields
  time += ONE_OVER_TWO<T> * dt;

  // update magnetic fields
  update_h(hxa, hya, hza);

  // half timestep update before updating electric fields
  time += ONE_OVER_TWO<T> * dt;

  // update electric fields
  update_e(ea, eb);
}

template <std::floating_point T>
void FDTDEngine<T>::update_e(const T ea, const T eb) {
  update_ex(ea, eb);
  update_ey(ea, eb);
  update_ez(ea, eb);
}

template <std::floating_point T>
void FDTDEngine<T>::update_h(const T hxa, const T hya, const T hza) {
  update_hx(hya, hza);
  update_hy(hxa, hza);
  update_hz(hya, hza);
}

template <std::floating_point T>
void FDTDEngine<T>::update_ex(const T ea, const T eb) {
  // assumes PEC outer boundary
  for (size_t i = 1; i < e.x.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.x.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.x.extent(2) - 1; ++k) {
        e.x[i, j, k] = ea * (eb * e.x[i, j, k] +
                             geom.d_inv.y * (h.z[i, j, k] - h.z[i, j - 1, k]) -
                             geom.d_inv.z * (h.y[i, j, k] - h.y[i, j, k - 1]));
      }
    }
  }
}

template <std::floating_point T>
void FDTDEngine<T>::update_ey(const T ea, const T eb) {
  // assumes PEC outer boundary
  for (size_t i = 1; i < e.y.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.y.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.y.extent(2) - 1; ++k) {
        e.y[i, j, k] = ea * (eb * e.y[i, j, k] +
                             geom.d_inv.z * (h.x[i, j, k] - h.x[i, j, k - 1]) -
                             geom.d_inv.x * (h.z[i, j, k] - h.z[i - 1, j, k]));
      }
    }
  }
}

template <std::floating_point T>
void FDTDEngine<T>::update_ez(const T ea, const T eb) {
  // assumes PEC outer boundary
  for (size_t i = 1; i < e.z.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.z.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.z.extent(2) - 1; ++k) {
        e.z[i, j, k] = ea * (eb * e.z[i, j, k] +
                             geom.d_inv.x * (h.y[i, j, k] - h.y[i - 1, j, k]) -
                             geom.d_inv.y * (h.x[i, j, k] - h.x[i, j - 1, k]));
      }
    }
  }
}

template <std::floating_point T>
void FDTDEngine<T>::update_hx(const T hya, const T hza) {
  // todo correctly implement for PEC boundary
  for (size_t i = 1; i < h.x.extent(0) - 1; ++i) {
    for (size_t j = 1; j < h.x.extent(1) - 1; ++j) {
      for (size_t k = 1; k < h.x.extent(2) - 1; ++k) {
        h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) +
                        hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
      }
    }
  }
}

template <std::floating_point T>
void FDTDEngine<T>::update_hy(const T hxa, const T hza) {
  // todo correctly implement for PEC boundary
  for (size_t i = 1; i < h.y.extent(0) - 1; ++i) {
    for (size_t j = 1; j < h.y.extent(1) - 1; ++j) {
      for (size_t k = 1; k < h.y.extent(2) - 1; ++k) {
        h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) +
                        hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
      }
    }
  }
}

template <std::floating_point T>
void FDTDEngine<T>::update_hz(const T hxa, const T hya) {
  // todo correctly implement for PEC boundary
  for (size_t i = 1; i < h.z.extent(0) - 1; ++i) {
    for (size_t j = 1; j < h.z.extent(1) - 1; ++j) {
      for (size_t k = 1; k < h.z.extent(2) - 1; ++k) {
        h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) +
                        hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
      }
    }
  }
}

// explicit template instantiation
template class FDTDGeometry<double>;
template class FDTDGeometry<float>;
template class FDTDEngine<double>;
template class FDTDEngine<float>;