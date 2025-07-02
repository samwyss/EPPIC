#include "fdtd_engine.h"

template <std::floating_point T>
FDTDGeometry<T>::FDTDGeometry(const Config<T> &config)
    : len({config.x_len, config.y_len, config.z_len}), ep_r(config.ep_r),
      mu_r(config.mu_r), ep(ep_r * VAC_PERMITTIVITY<T>),
      mu(mu_r * VAC_PERMEABILITY<T>), sigma(config.sigma) {

  SPDLOG_TRACE("enter FDTDGeometry<T>::FDTDGeometry");
  SPDLOG_DEBUG("bounding box (m): {:.3e} x {:.3e} x {:.3e}", len.x, len.y,
               len.z);

  // (m) maximum spatial step based on maximum frequency
  const T ds_min_wavelength =
      VAC_SPEED_OF_LIGHT<T> /
      (sqrt(ep_r * mu_r) * static_cast<T>(config.num_vox_min_wavelength) *
       config.max_frequency);
  SPDLOG_DEBUG("maximum spatial step based on maximum frequency (m): {:.3e}",
               ds_min_wavelength);

  // (m) maximum spatial step based on minimum feature size
  const T ds_min_feature_size = std::min({len.x, len.y, len.z}) /
                                static_cast<T>(config.num_vox_min_feature);
  SPDLOG_DEBUG("maximum spatial step based on feature size (m): {:.3e}",
               ds_min_feature_size);

  // (m) maximum required spatial step
  const T ds = std::min({ds_min_wavelength, ds_min_feature_size});
  SPDLOG_DEBUG("maximum spatial step (m): {:.3e}", ds);

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(static_cast<double>(len.x) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.y) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.z) / ds))};
  SPDLOG_DEBUG("field voxel dimensions: {} x {} x {}", nv.x, nv.y, nv.z);
  SPDLOG_DEBUG("voxels to update each step: {}", 6 * nv.x);

  // (m) final spatial steps
  d = {len.x / static_cast<T>(nv.x), len.y / static_cast<T>(nv.y),
       len.z / static_cast<T>(nv.z)};
  SPDLOG_DEBUG("voxel size (m): {:.3e} x {:.3e} x {:.3e}", d.x, d.y, d.z);

  // (m^-1) inverse spatial steps
  d_inv = {static_cast<T>(1.0) / d.x, static_cast<T>(1.0) / d.y,
           static_cast<T>(1.0) / d.z};
  SPDLOG_DEBUG("inverse voxel size (m) , {:.3e} x {:.3e} x {:.3e}", d_inv.x,
               d_inv.y, d_inv.z);

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
    // todo why not const like FDTDGeometry?
    FDTDEngine engine(config);
    SPDLOG_TRACE("exit FDTDEngine<T>::create with success");
    return engine;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit FDTDEngine<T>::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
uint64_t FDTDEngine<T>::calc_num_steps(const T adv_t) const {
  SPDLOG_TRACE("enter FDTDEngine<T>::calc_num_steps");

  // find maximum number of timesteps required for any solver
  const uint64_t max_num_steps = calc_cfl_steps(adv_t);
  SPDLOG_DEBUG("maximum number of steps required by any solver: {}",
               max_num_steps);

  SPDLOG_TRACE("exit FDTDEngine<T>::calc_num_steps");
  return max_num_steps;
}

template <std::floating_point T>
uint64_t FDTDEngine<T>::calc_cfl_steps(const T time_span) const {
  SPDLOG_TRACE("enter FDTDEngine<T>::calc_cfl_steps");

  const T maximum_dt =
      1.0 / (VAC_SPEED_OF_LIGHT<T> / sqrt(geom.ep_r * geom.mu_r) *
             sqrt(pow(geom.d_inv.x, 2) + pow(geom.d_inv.y, 2) +
                  pow(geom.d_inv.z, 2)));
  SPDLOG_DEBUG("maximum possible timestep to satisfy CFL condition (s): {:.3e}",
               maximum_dt);

  const T num_steps = static_cast<uint64_t>(ceil(time_span / maximum_dt));
  SPDLOG_DEBUG("steps required to satisfy CFL condition: {}", num_steps);

  SPDLOG_TRACE("exit FDTDEngine<T>::calc_cfl_steps");
  return num_steps;
}

template <std::floating_point T> void FDTDEngine<T>::step(const T dt) {
  SPDLOG_TRACE("enter FDTDEngine<T>::step");

  // TODO it would be nice to not need to recalculate these every time step is
  // TODO called, the performance penalty of this has yet to be assed however

  // electric field a loop constant
  const T ea = 1.0 / (geom.ep / dt + geom.sigma / 2.0);
  SPDLOG_TRACE("ea loop constant: {:.3e}", ea);

  // electric field b loop constant
  const T eb = geom.ep / dt - geom.sigma / 2.0;
  SPDLOG_TRACE("eb loop constant: {:.3e}", eb);

  // magnetic field a loop constant for x-component
  const T hxa = dt * geom.d_inv.x / geom.mu;
  SPDLOG_TRACE("hxa loop constant: {:.3e}", hxa);

  // magnetic field a loop constant for y-component
  const T hya = dt * geom.d_inv.y / geom.mu;
  SPDLOG_TRACE("hya loop constant: {:.3e}", hya);

  // magnetic field a loop constant for z-component
  const T hza = dt * geom.d_inv.z / geom.mu;
  SPDLOG_TRACE("hza loop constant: {:.3e}", hza);

  // half timestep update before updating magnetic fields
  time += ONE_OVER_TWO<T> * dt;
  SPDLOG_TRACE("advance half time step to (s): {:.5e}", time);

  // update magnetic fields
  update_h(hxa, hya, hza);

  // half timestep update before updating electric fields
  time += ONE_OVER_TWO<T> * dt;
  SPDLOG_TRACE("advance half time step to (s): {:.5e}", time);

  // update electric fields
  update_e(ea, eb);

  SPDLOG_TRACE("exit FDTDEngine<T>::step");
}

template <std::floating_point T>
uint64_t FDTDEngine<T>::get_field_num_vox() const {
  SPDLOG_TRACE("enter FDTDEngine<T>::get_field_num_vox");

  const auto num_vox = e.x.size();

  SPDLOG_TRACE("exit FDTDEngine<T>::get_field_num_vox");
  return num_vox;
}

template <std::floating_point T>
void FDTDEngine<T>::update_e(const T ea, const T eb) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_e");

  update_ex(ea, eb);
  update_ey(ea, eb);
  update_ez(ea, eb);

  SPDLOG_TRACE("exit FDTDEngine<T>::update_e");
}

template <std::floating_point T>
void FDTDEngine<T>::update_h(const T hxa, const T hya, const T hza) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_h");

  update_hx(hya, hza);
  update_hy(hxa, hza);
  update_hz(hxa, hya);

  SPDLOG_TRACE("exit FDTDEngine<T>::update_h");
}

template <std::floating_point T>
void FDTDEngine<T>::update_ex(const T ea, const T eb) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_ex");

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

  SPDLOG_TRACE("exit FDTDEngine<T>::update_ex");
}

template <std::floating_point T>
void FDTDEngine<T>::update_ey(const T ea, const T eb) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_ey");

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

  SPDLOG_TRACE("exit FDTDEngine<T>::update_ey");
}

template <std::floating_point T>
void FDTDEngine<T>::update_ez(const T ea, const T eb) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_ez");

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

  SPDLOG_TRACE("exit FDTDEngine<T>::update_ez");
}

template <std::floating_point T>
void FDTDEngine<T>::update_hx(const T hya, const T hza) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_hx");

  // todo remove branches
  for (size_t i = 0; i < h.x.extent(0); ++i) {
    for (size_t j = 0; j < h.x.extent(1); ++j) {
      for (size_t k = 0; k < h.x.extent(2); ++k) {
        [[likely]] if (j != h.x.extent(1) - 1 && k != h.x.extent(2) - 1) {
          // j- k-low volume
          h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) +
                          hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
        } else if (j != h.x.extent(1) - 1 && k == h.x.extent(2) - 1) {
          // k-high plane
          h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) +
                          hza * (0.0 - e.y[i, j, k]);
        } else if (j == h.x.extent(1) - 1 && k != h.x.extent(2) - 1) {
          // j-high plane
          h.x[i, j, k] += -hya * (0.0 - e.z[i, j, k]) +
                          hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
        } else {
          // j- k-high line
          h.x[i, j, k] +=
              -hya * (0.0 - e.z[i, j, k]) + hza * (0.0 - e.y[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine<T>::update_hx");
}

template <std::floating_point T>
void FDTDEngine<T>::update_hy(const T hxa, const T hza) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_hy");

  // todo remove branches
  for (size_t i = 0; i < h.y.extent(0); ++i) {
    for (size_t j = 0; j < h.y.extent(1); ++j) {
      for (size_t k = 0; k < h.y.extent(2); ++k) {
        [[likely]] if (i != h.y.extent(0) - 1 && k != h.y.extent(2) - 1) {
          // i- k-low volume
          h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) +
                          hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
        } else if (i != h.y.extent(0) - 1 && k == h.y.extent(2) - 1) {
          // k-high plane
          h.y[i, j, k] += -hza * (0.0 - e.x[i, j, k]) +
                          hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && k != h.y.extent(2) - 1) {
          // i-high plane
          h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) +
                          hxa * (0.0 - e.z[i, j, k]);
        } else {
          // i- k-high line
          h.y[i, j, k] +=
              -hza * (0.0 - e.x[i, j, k]) + hxa * (0.0 - e.z[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine<T>::update_hy");
}

template <std::floating_point T>
void FDTDEngine<T>::update_hz(const T hxa, const T hya) {
  SPDLOG_TRACE("enter FDTDEngine<T>::update_hz");

  // todo correctly implement for PEC boundary
  // todo remove branches
  for (size_t i = 0; i < h.z.extent(0); ++i) {
    for (size_t j = 0; j < h.z.extent(1); ++j) {
      for (size_t k = 0; k < h.z.extent(2); ++k) {
        [[likely]] if (i != h.y.extent(0) - 1 && j != h.y.extent(1) - 1) {
          // i- j-low volume
          h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) +
                          hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
        } else if (i != h.y.extent(0) - 1 && j == h.y.extent(1) - 1) {
          // j-high plane
          h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) +
                          hya * (0.0 - e.x[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && j != h.y.extent(1) - 1) {
          // i-high plane
          h.z[i, j, k] += -hxa * (0.0 - e.y[i, j, k]) +
                          hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
        } else {
          // i- j-high line
          h.z[i, j, k] +=
              -hxa * (0.0 - e.y[i, j, k]) + hya * (0.0 - e.x[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine<T>::update_hz");
}

// explicit template instantiation
template class FDTDGeometry<double>;
template class FDTDGeometry<float>;
template class FDTDEngine<double>;
template class FDTDEngine<float>;