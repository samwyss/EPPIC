#include "fdtd_engine.h"

FDTDEngine::FDTDEngine(const Config &config)
    : len({config.x_len, config.y_len, config.z_len}), ep_r(config.ep_r),
      mu_r(config.mu_r), ep(ep_r * VAC_PERMITTIVITY),
      mu(mu_r * VAC_PERMEABILITY), sigma(config.sigma) {
  SPDLOG_TRACE("enter FDTDEngine::FDTDEngine");
  SPDLOG_DEBUG("bounding box (m): {:.3e} x {:.3e} x {:.3e}", len.x, len.y,
               len.z);

  // (m) maximum spatial step based on maximum frequency
  const fpp ds_min_wavelength =
      VAC_SPEED_OF_LIGHT /
      static_cast<fpp>(sqrt(ep_r * mu_r) *
                       static_cast<fpp>(config.num_vox_min_wavelength) *
                       config.max_frequency);
  SPDLOG_DEBUG("maximum spatial step based on maximum frequency (m): {:.3e}",
               ds_min_wavelength);

  // (m) maximum spatial step based on minimum feature size
  const fpp ds_min_feature_size = std::min({len.x, len.y, len.z}) /
                                  static_cast<fpp>(config.num_vox_min_feature);
  SPDLOG_DEBUG("maximum spatial step based on feature size (m): {:.3e}",
               ds_min_feature_size);

  // (m) maximum required spatial step
  const fpp ds = std::min({ds_min_wavelength, ds_min_feature_size});
  SPDLOG_DEBUG("maximum spatial step (m): {:.3e}", ds);

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(static_cast<double>(len.x) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.y) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(len.z) / ds))};
  SPDLOG_DEBUG("field voxel dimensions: {} x {} x {}", nv.x, nv.y, nv.z);
  SPDLOG_DEBUG("voxels to update each step: {}", 6 * nv.x);

  // (m) final spatial steps
  d = {len.x / static_cast<fpp>(nv.x), len.y / static_cast<fpp>(nv.y),
       len.z / static_cast<fpp>(nv.z)};
  SPDLOG_DEBUG("voxel size (m): {:.3e} x {:.3e} x {:.3e}", d.x, d.y, d.z);

  // (m^-1) inverse spatial steps
  d_inv = {static_cast<fpp>(1.0) / d.x, static_cast<fpp>(1.0) / d.y,
           static_cast<fpp>(1.0) / d.z};
  SPDLOG_DEBUG("inverse voxel size (m^-1) , {:.3e} x {:.3e} x {:.3e}", d_inv.x,
               d_inv.y, d_inv.z);

  // initialize fields
  e = Vector3<fpp>(nv, 0.0);
  h = Vector3<fpp>(nv, 0.0);

  SPDLOG_TRACE("exit FDTDEngine::FDTDEngine");
}

std::expected<FDTDEngine, std::string>
FDTDEngine::create(const Config &config) {
  SPDLOG_TRACE("enter FDTDEngine::create");
  try {
    // todo why not const like FDTDGeometry?
    FDTDEngine engine(config);
    SPDLOG_TRACE("exit FDTDEngine::create with success");
    return engine;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit FDTDEngine::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

uint64_t FDTDEngine::calc_num_steps(const fpp adv_t) const {
  SPDLOG_TRACE("enter FDTDEngine::calc_num_steps");

  // find maximum number of timesteps required for any solver
  const uint64_t max_num_steps = calc_cfl_steps(adv_t);
  SPDLOG_DEBUG("maximum number of steps required by any solver: {}",
               max_num_steps);

  SPDLOG_TRACE("exit FDTDEngine::calc_num_steps");
  return max_num_steps;
}

uint64_t FDTDEngine::calc_cfl_steps(const fpp time_span) const {
  SPDLOG_TRACE("enter FDTDEngine::calc_cfl_steps");

  const fpp maximum_dt = static_cast<fpp>(
      1.0 / (VAC_SPEED_OF_LIGHT / sqrt(ep_r * mu_r) *
             sqrt(pow(d_inv.x, 2) + pow(d_inv.y, 2) + pow(d_inv.z, 2))));
  SPDLOG_DEBUG("maximum possible timestep to satisfy CFL condition (s): {:.3e}",
               maximum_dt);

  const auto num_steps = static_cast<uint64_t>(ceil(time_span / maximum_dt));
  SPDLOG_DEBUG("steps required to satisfy CFL condition: {}", num_steps);

  SPDLOG_TRACE("exit FDTDEngine::calc_cfl_steps");
  return num_steps;
}

void FDTDEngine::step(const fpp dt) {
  SPDLOG_TRACE("enter FDTDEngine::step");

  // TODO it would be nice to not need to recalculate these every time step is
  // TODO called, the performance penalty of this has yet to be assed however

  // electric field a loop constant
  const auto ea =
      static_cast<fpp>(1.0) / (ep / dt + sigma / static_cast<fpp>(2.0));
  SPDLOG_TRACE("ea loop constant: {:.3e}", ea);

  // electric field b loop constant
  const auto eb = ep / dt - sigma / static_cast<fpp>(2.0);
  SPDLOG_TRACE("eb loop constant: {:.3e}", eb);

  // magnetic field a loop constant for x-component
  const auto hxa = dt * d_inv.x / mu;
  SPDLOG_TRACE("hxa loop constant: {:.3e}", hxa);

  // magnetic field a loop constant for y-component
  const auto hya = dt * d_inv.y / mu;
  SPDLOG_TRACE("hya loop constant: {:.3e}", hya);

  // magnetic field a loop constant for z-component
  const auto hza = dt * d_inv.z / mu;
  SPDLOG_TRACE("hza loop constant: {:.3e}", hza);

  // half timestep update before updating magnetic fields
  time += ONE_OVER_TWO * dt;
  SPDLOG_TRACE("advance half time step to (s): {:.5e}", time);

  // update magnetic fields
  update_h(hxa, hya, hza);

  // half timestep update before updating electric fields
  time += ONE_OVER_TWO * dt;
  SPDLOG_TRACE("advance half time step to (s): {:.5e}", time);

  // update electric fields
  update_e(ea, eb);

  SPDLOG_TRACE("exit FDTDEngine::step");
}

uint64_t FDTDEngine::get_field_num_vox() const {
  SPDLOG_TRACE("enter FDTDEngine::get_field_num_vox");

  const auto num_vox = e.x.size();

  SPDLOG_TRACE("exit FDTDEngine::get_field_num_vox");
  return num_vox;
}

void FDTDEngine::update_e(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter FDTDEngine::update_e");

  update_ex(ea, eb);
  update_ey(ea, eb);
  update_ez(ea, eb);

  SPDLOG_TRACE("exit FDTDEngine::update_e");
}

void FDTDEngine::update_h(const fpp hxa, const fpp hya, const fpp hza) const {
  SPDLOG_TRACE("enter FDTDEngine::update_h");

  update_hx(hya, hza);
  update_hy(hxa, hza);
  update_hz(hxa, hya);

  SPDLOG_TRACE("exit FDTDEngine::update_h");
}

void FDTDEngine::update_ex(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter FDTDEngine::update_ex");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.x.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.x.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.x.extent(2) - 1; ++k) {
        e.x[i, j, k] = ea * (eb * e.x[i, j, k] +
                             d_inv.y * (h.z[i, j, k] - h.z[i, j - 1, k]) -
                             d_inv.z * (h.y[i, j, k] - h.y[i, j, k - 1]));
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_ex");
}

void FDTDEngine::update_ey(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter FDTDEngine::update_ey");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.y.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.y.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.y.extent(2) - 1; ++k) {
        e.y[i, j, k] = ea * (eb * e.y[i, j, k] +
                             d_inv.z * (h.x[i, j, k] - h.x[i, j, k - 1]) -
                             d_inv.x * (h.z[i, j, k] - h.z[i - 1, j, k]));
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_ey");
}

void FDTDEngine::update_ez(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter FDTDEngine::update_ez");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.z.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.z.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.z.extent(2) - 1; ++k) {
        e.z[i, j, k] = ea * (eb * e.z[i, j, k] +
                             d_inv.x * (h.y[i, j, k] - h.y[i - 1, j, k]) -
                             d_inv.y * (h.x[i, j, k] - h.x[i, j - 1, k]));
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_ez");
}

void FDTDEngine::update_hx(const fpp hya, const fpp hza) const {
  SPDLOG_TRACE("enter FDTDEngine::update_hx");

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
                          hza * (static_cast<fpp>(0.0) - e.y[i, j, k]);
        } else if (j == h.x.extent(1) - 1 && k != h.x.extent(2) - 1) {
          // j-high plane
          h.x[i, j, k] += -hya * (static_cast<fpp>(0.0) - e.z[i, j, k]) +
                          hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
        } else {
          // j- k-high line
          h.x[i, j, k] += -hya * (static_cast<fpp>(0.0) - e.z[i, j, k]) +
                          hza * (static_cast<fpp>(0.0) - e.y[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_hx");
}

void FDTDEngine::update_hy(const fpp hxa, const fpp hza) const {
  SPDLOG_TRACE("enter FDTDEngine::update_hy");

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
          h.y[i, j, k] += -hza * (static_cast<fpp>(0.0) - e.x[i, j, k]) +
                          hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && k != h.y.extent(2) - 1) {
          // i-high plane
          h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) +
                          hxa * (static_cast<fpp>(0.0) - e.z[i, j, k]);
        } else {
          // i- k-high line
          h.y[i, j, k] += -hza * (static_cast<fpp>(0.0) - e.x[i, j, k]) +
                          hxa * (static_cast<fpp>(0.0) - e.z[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_hy");
}

void FDTDEngine::update_hz(const fpp hxa, const fpp hya) const {
  SPDLOG_TRACE("enter FDTDEngine::update_hz");

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
                          hya * (static_cast<fpp>(0.0) - e.x[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && j != h.y.extent(1) - 1) {
          // i-high plane
          h.z[i, j, k] += -hxa * (static_cast<fpp>(0.0) - e.y[i, j, k]) +
                          hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
        } else {
          // i- j-high line
          h.z[i, j, k] += -hxa * (static_cast<fpp>(0.0) - e.y[i, j, k]) +
                          hya * (static_cast<fpp>(0.0) - e.x[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit FDTDEngine::update_hz");
}