#include "world.h"

World::World(const std::string &input_file_path, const std::string &id)
    : cfg(input_file_path, id), h5(init_h5()), ep(cfg.ep_r * VAC_PERMITTIVITY), mu(cfg.mu_r * VAC_PERMEABILITY),
      nv_e(init_nv_e()), nv_h(init_nv_h()), d(init_d()), d_inv(init_d_inv()), e(Vector3(nv_e, static_cast<fpp>(0.0))),
      h(Vector3(nv_h, static_cast<fpp>(0.0))) {}

HDF5Obj World::init_h5() const {
  SPDLOG_TRACE("enter World::init_h5");
  HDF5Obj h5(H5Fcreate((cfg.out / "data.h5").c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT), H5Fclose);
  SPDLOG_DEBUG("created output HDF5 file at `{}`", (cfg.out / "data.h5").string());

  SPDLOG_TRACE("exit World::init_h5");
  return h5;
}

Coord3<size_t> World::init_nv_h() const {
  SPDLOG_TRACE("enter World::init_nv_h");

  // (m) maximum spatial step based on maximum frequency
  const fpp ds_min_wavelength =
      VAC_SPEED_OF_LIGHT /
      static_cast<fpp>(sqrt(cfg.ep_r * cfg.mu_r) * static_cast<fpp>(cfg.num_vox_min_wavelength) * cfg.max_frequency);
  SPDLOG_DEBUG("maximum spatial step based on maximum frequency (m): {:.3e}", ds_min_wavelength);

  // (m) maximum spatial step based on minimum feature size
  const fpp ds_min_feature_size =
      std::min({cfg.len.x, cfg.len.y, cfg.len.z}) / static_cast<fpp>(cfg.num_vox_min_feature);
  SPDLOG_DEBUG("maximum spatial step based on feature size (m): {:.3e}", ds_min_feature_size);

  // (m) maximum required spatial step
  const fpp ds = std::min({ds_min_wavelength, ds_min_feature_size});
  SPDLOG_DEBUG("maximum spatial step (m): {:.3e}", ds);

  // the computation here is a result of snapping the maximum step to the geometry
  const Coord3 nv_h = {static_cast<size_t>(ceil(static_cast<double>(cfg.len.x) / ds)),
                       static_cast<size_t>(ceil(static_cast<double>(cfg.len.y) / ds)),
                       static_cast<size_t>(ceil(static_cast<double>(cfg.len.z) / ds))};

  SPDLOG_DEBUG("magnetic field voxel dimensions: {} x {} x {}", nv_h.x, nv_h.y, nv_h.z);

  SPDLOG_TRACE("exit World::init_nv_h");
  return nv_h;
}

Coord3<size_t> World::init_nv_e() const {
  SPDLOG_TRACE("enter World::init_nv_e");

  // the +1 is a result of the convention that all magnetic field points are wrapped by an electric field
  const Coord3 nv_e = {nv_h.x + 1, nv_h.y + 1, nv_h.z + 1};

  SPDLOG_DEBUG("electric field voxel dimensions: {} x {} x {}", nv_e.x, nv_e.y, nv_e.z);
  SPDLOG_DEBUG("voxels to update each step: {}", 3 * (nv_e.x * nv_e.y * nv_e.z + nv_h.x * nv_h.y * nv_h.z));

  SPDLOG_TRACE("exit World::init_nv_e");
  return nv_e;
}

Coord3<fpp> World::init_d() const {
  SPDLOG_TRACE("enter World::init_d");

  // the magnetic field numbers are used as a result of the aforementioned magnetic field wrapping of the electric field
  // the math works out nicely this way
  const Coord3 d = {cfg.len.x / static_cast<fpp>(nv_h.x), cfg.len.y / static_cast<fpp>(nv_h.y),
                    cfg.len.z / static_cast<fpp>(nv_h.z)};
  SPDLOG_DEBUG("voxel size (m): {:.3e} x {:.3e} x {:.3e}", d.x, d.y, d.z);

  SPDLOG_TRACE("exit World::init_d");
  return d;
}

Coord3<fpp> World::init_d_inv() const {
  SPDLOG_TRACE("enter World::init_d_inv");

  const Coord3 d_inv = {static_cast<fpp>(1.0) / d.x, static_cast<fpp>(1.0) / d.y, static_cast<fpp>(1.0) / d.z};
  SPDLOG_DEBUG("inverse voxel size (m^-1): {:.3e} x {:.3e} x {:.3e}", d_inv.x, d_inv.y, d_inv.z);

  SPDLOG_TRACE("exit World::init_d_inv");
  return d_inv;
}

std::expected<void, std::string> World::run() {
  SPDLOG_TRACE("enter World::run");
  SPDLOG_DEBUG("running EPPIC to end time of {:.3e} (s)", cfg.end_time);

  if (const auto result = advance_to(cfg.end_time); !result.has_value()) {
    SPDLOG_CRITICAL("failed to run EPPIC to desired end time: {}", result.error());
    return std::unexpected(result.error());
  }

  SPDLOG_TRACE("exit World::run with success");
  return {};
}

std::expected<void, std::string> World::advance_to(const fpp end_t) {
  SPDLOG_TRACE("enter World::advance_to");
  SPDLOG_DEBUG("current time (s): {:.3e}", time);
  SPDLOG_DEBUG("advance time to (s):  {:.3e}", end_t);

  if (end_t > time) {
    const fpp adv_t = end_t - time;

    if (const auto result = advance_by(adv_t); !result.has_value()) {
      SPDLOG_CRITICAL("failed to advance time to {} (s): {}", adv_t, result.error());
      return std::unexpected(result.error());
    }

  } else {
    SPDLOG_WARN("end time of {:.3e} (s) is not greater than current time of {:.3e} (s)", end_t, time);
  }

  SPDLOG_TRACE("exit World::advance_to with success");
  return {};
}

std::expected<void, std::string> World::advance_by(const fpp adv_t) {
  SPDLOG_TRACE("enter World::advance_by");
  SPDLOG_DEBUG("advance time by (s): {:.3e}", adv_t);

  // (s) initial time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  [[maybe_unused]] const auto init_time = time;

  // number of steps required to satisfy most stringent requirement
  const auto steps = calc_num_steps(adv_t);

  // (s) time step
  const fpp dt = adv_t / static_cast<fpp>(steps);
  SPDLOG_DEBUG("timestep (s): {:.3e}", dt);

  const uint64_t logged_steps = steps / cfg.ds_ratio + 1;

  const auto metadata_group = HDF5Obj(H5Gcreate(h5.get(), "metadata", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose);
  write_metadata(metadata_group, dt, logged_steps);

  const auto data_group = HDF5Obj(H5Gcreate(h5.get(), "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose);
  setup_datasets(data_group, logged_steps);

  // loop start time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto start_time = std::chrono::high_resolution_clock::now();

  // main time loop
  SPDLOG_DEBUG("enter main time loop");
  try {
    for (uint64_t i = 0; i < steps; ++i) {
      SPDLOG_DEBUG("step: {}/{} elapsed time (s): {:.5e}/{:.5e}", i + 1, steps, time, init_time + adv_t);

      // advance by one step
      step(dt);

      if (0 == i % cfg.ds_ratio || i == steps - 1) [[unlikely]] {
        SPDLOG_DEBUG("begin data logging");

        // hyperslab index to write to
        uint64_t hyperslab = i / cfg.ds_ratio;

        // case for the last timestep
        if (i == steps - 1) [[unlikely]] {
          hyperslab++;
        }

        SPDLOG_DEBUG("hyperslab index: {}/{}", hyperslab, logged_steps);

        // write_time(group);
        // write_fields(group);

        SPDLOG_DEBUG("end data logging");
      }
    }
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("main time loop returned with error: {}", err.what());
    return std::unexpected(err.what());
  }
  SPDLOG_DEBUG("exit main time loop with success");

  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto end_time = std::chrono::high_resolution_clock::now();
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto num_cells = 3 * (e.x.size() + h.x.size()) * steps;
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto loop_time = end_time - start_time;
  SPDLOG_INFO("loop runtime: {:%H:%M:%S}", loop_time);
  SPDLOG_INFO("voxel compute rate (vox/s): {:.3e}",
              static_cast<double>(num_cells) / std::chrono::duration<double>(loop_time).count());

  SPDLOG_TRACE("exit World::advance_by");

  return {};
}

std::filesystem::path World::get_output_dir() const { return cfg.out; }

uint64_t World::calc_num_steps(const fpp adv_t) const {
  SPDLOG_TRACE("enter World::calc_num_steps");

  // find maximum number of timesteps required for any solver
  const uint64_t max_num_steps = calc_cfl_steps(adv_t);
  SPDLOG_DEBUG("maximum number of steps required by any solver: {}", max_num_steps);

  SPDLOG_TRACE("exit World::calc_num_steps");
  return max_num_steps;
}

uint64_t World::calc_cfl_steps(const fpp time_span) const {
  SPDLOG_TRACE("enter World::calc_cfl_steps");

  const fpp maximum_dt = static_cast<fpp>(1.0 / (VAC_SPEED_OF_LIGHT / sqrt(cfg.ep_r * cfg.mu_r) *
                                                 sqrt(pow(d_inv.x, 2) + pow(d_inv.y, 2) + pow(d_inv.z, 2))));
  SPDLOG_DEBUG("maximum possible timestep to satisfy CFL condition (s): {:.3e}", maximum_dt);

  const auto num_steps = static_cast<uint64_t>(ceil(time_span / maximum_dt));
  SPDLOG_DEBUG("steps required to satisfy CFL condition: {}", num_steps);

  SPDLOG_TRACE("exit World::calc_cfl_steps");
  return num_steps;
}

void World::step(const fpp dt) {
  SPDLOG_TRACE("enter World::step");

  // TODO it would be nice to not need to recalculate these every time step is
  // TODO called, the performance penalty of this has yet to be assed however

  // electric field a loop constant
  const auto ea = static_cast<fpp>(1.0) / (ep / dt + cfg.sigma / static_cast<fpp>(2.0));
  SPDLOG_TRACE("ea loop constant: {:.3e}", ea);

  // electric field b loop constant
  const auto eb = ep / dt - cfg.sigma / static_cast<fpp>(2.0);
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

  SPDLOG_TRACE("exit World::step");
}

void World::update_e(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter World::update_e");

  update_ex(ea, eb);
  update_ey(ea, eb);
  update_ez(ea, eb);

  SPDLOG_TRACE("exit World::update_e");
}

void World::update_h(const fpp hxa, const fpp hya, const fpp hza) const {
  SPDLOG_TRACE("enter World::update_h");

  update_hx(hya, hza);
  update_hy(hxa, hza);
  update_hz(hxa, hya);

  SPDLOG_TRACE("exit World::update_h");
}

void World::update_ex(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter World::update_ex");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.x.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.x.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.x.extent(2) - 1; ++k) {
        e.x[i, j, k] = ea * (eb * e.x[i, j, k] + d_inv.y * (h.z[i, j, k] - h.z[i, j - 1, k]) -
                             d_inv.z * (h.y[i, j, k] - h.y[i, j, k - 1]));
      }
    }
  }

  SPDLOG_TRACE("exit World::update_ex");
}

void World::update_ey(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter World::update_ey");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.y.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.y.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.y.extent(2) - 1; ++k) {
        e.y[i, j, k] = ea * (eb * e.y[i, j, k] + d_inv.z * (h.x[i, j, k] - h.x[i, j, k - 1]) -
                             d_inv.x * (h.z[i, j, k] - h.z[i - 1, j, k]));
      }
    }
  }

  SPDLOG_TRACE("exit World::update_ey");
}

void World::update_ez(const fpp ea, const fpp eb) const {
  SPDLOG_TRACE("enter World::update_ez");

  // assumes PEC outer boundary
  for (size_t i = 1; i < e.z.extent(0) - 1; ++i) {
    for (size_t j = 1; j < e.z.extent(1) - 1; ++j) {
      for (size_t k = 1; k < e.z.extent(2) - 1; ++k) {
        e.z[i, j, k] = ea * (eb * e.z[i, j, k] + d_inv.x * (h.y[i, j, k] - h.y[i - 1, j, k]) -
                             d_inv.y * (h.x[i, j, k] - h.x[i, j - 1, k]));
      }
    }
  }

  SPDLOG_TRACE("exit World::update_ez");
}

void World::update_hx(const fpp hya, const fpp hza) const {
  SPDLOG_TRACE("enter World::update_hx");

  for (size_t i = 0; i < h.x.extent(0); ++i) {
    for (size_t j = 0; j < h.x.extent(1); ++j) {
      for (size_t k = 0; k < h.x.extent(2); ++k) {
        h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) + hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
      }
    }
  }

  SPDLOG_TRACE("exit World::update_hx");
}

void World::update_hy(const fpp hxa, const fpp hza) const {
  SPDLOG_TRACE("enter World::update_hy");

  for (size_t i = 0; i < h.y.extent(0); ++i) {
    for (size_t j = 0; j < h.y.extent(1); ++j) {
      for (size_t k = 0; k < h.y.extent(2); ++k) {
        h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) + hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
      }
    }
  }

  SPDLOG_TRACE("exit World::update_hy");
}

void World::update_hz(const fpp hxa, const fpp hya) const {
  SPDLOG_TRACE("enter World::update_hz");

  for (size_t i = 0; i < h.z.extent(0); ++i) {
    for (size_t j = 0; j < h.z.extent(1); ++j) {
      for (size_t k = 0; k < h.z.extent(2); ++k) {
        h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) + hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
      }
    }
  }

  SPDLOG_TRACE("exit World::update_hz");
}

void World::log(uint64_t hyperslab) const {
  SPDLOG_TRACE("enter World::log");

  SPDLOG_TRACE("exit World::log");
}

void World::write_fields(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::h5_write_field");

  const hsize_t dims_e[3] = {static_cast<hsize_t>(nv_e.x), static_cast<hsize_t>(nv_e.y), static_cast<hsize_t>(nv_e.z)};
  const hsize_t dims_h[3] = {static_cast<hsize_t>(nv_h.x), static_cast<hsize_t>(nv_h.y), static_cast<hsize_t>(nv_h.z)};

  const auto dspace_e = HDF5Obj(H5Screate_simple(3, dims_e, nullptr), H5Sclose);
  const auto dspace_h = HDF5Obj(H5Screate_simple(3, dims_h, nullptr), H5Sclose);

  const auto ex =
      HDF5Obj(H5Dcreate(group.get(), "ex", h5_fpp, dspace_e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto ey =
      HDF5Obj(H5Dcreate(group.get(), "ey", h5_fpp, dspace_e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto ez =
      HDF5Obj(H5Dcreate(group.get(), "ez", h5_fpp, dspace_e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto hx =
      HDF5Obj(H5Dcreate(group.get(), "hx", h5_fpp, dspace_h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto hy =
      HDF5Obj(H5Dcreate(group.get(), "hy", h5_fpp, dspace_h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto hz =
      HDF5Obj(H5Dcreate(group.get(), "hz", h5_fpp, dspace_h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

  H5Dwrite(ex.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.x.data_handle());
  H5Dwrite(ey.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.y.data_handle());
  H5Dwrite(ez.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.z.data_handle());
  H5Dwrite(hx.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.x.data_handle());
  H5Dwrite(hy.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.y.data_handle());
  H5Dwrite(hz.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.z.data_handle());

  SPDLOG_TRACE("exit World::h5_write_field");
}

void World::write_time(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_time");

  constexpr hsize_t dims[1] = {1};
  const auto dspace = HDF5Obj(H5Screate_simple(1, dims, nullptr), H5Sclose);
  const auto time =
      HDF5Obj(H5Dcreate(group.get(), "time", h5_fpp, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  H5Dwrite(time.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, &this->time);

  SPDLOG_TRACE("exit World::write_time");
}

void World::write_metadata(const HDF5Obj &group, const double dt, const uint64_t num) const {
  SPDLOG_TRACE("enter World::write_metadata");

  const fpp delta_t[1] = {dt};
  const fpp dxdydz[3] = {d.x, d.y, d.z};
  const uint64_t num_logs[1] = {num};

  constexpr hsize_t dims_scalar[1] = {1};
  constexpr hsize_t dims_xyz[1] = {3};

  const auto dspace_scalar = HDF5Obj(H5Screate_simple(1, dims_scalar, nullptr), H5Sclose);
  const auto dspace_xyz = HDF5Obj(H5Screate_simple(1, dims_xyz, nullptr), H5Sclose);

  const auto timestep = HDF5Obj(
      H5Dcreate(group.get(), "dt", h5_fpp, dspace_scalar.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto spacing = HDF5Obj(
      H5Dcreate(group.get(), "dxdydz", h5_fpp, dspace_xyz.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  const auto number_logs = HDF5Obj(H5Dcreate(group.get(), "logged_steps", H5T_NATIVE_UINT64, dspace_scalar.get(),
                                             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                                   H5Dclose);

  H5Dwrite(timestep.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, &delta_t);
  H5Dwrite(spacing.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, &dxdydz);
  H5Dwrite(number_logs.get(), h5_fpp, H5S_ALL, H5S_ALL, H5P_DEFAULT, &num_logs);

  SPDLOG_TRACE("exit World::write_metadata");
}

void World::setup_dataspaces(const uint64_t num) {
  SPDLOG_TRACE("enter World::setup_dataspaces");

  const hsize_t dims_scalar[1] = {num};
  const hsize_t dims_e[4] = {static_cast<hsize_t>(nv_e.x), static_cast<hsize_t>(nv_e.y), static_cast<hsize_t>(nv_e.z),
                             static_cast<hsize_t>(num)};
  const hsize_t dims_h[4] = {static_cast<hsize_t>(nv_h.x), static_cast<hsize_t>(nv_h.y), static_cast<hsize_t>(nv_h.z),
                             static_cast<hsize_t>(num)};

  dataspaces.scalar = HDF5Obj(H5Screate_simple(1, dims_scalar, nullptr), H5Sclose);
  dataspaces.e = HDF5Obj(H5Screate_simple(4, dims_e, nullptr), H5Sclose);
  dataspaces.h = HDF5Obj(H5Screate_simple(4, dims_h, nullptr), H5Sclose);

  SPDLOG_TRACE("exit World::setup_dataspaces");
}

void World::setup_datasets(const HDF5Obj &group, const uint64_t num) {
  SPDLOG_TRACE("enter World::setup_datasets");

  datasets.time = HDF5Obj(
      H5Dcreate(group.get(), "time", h5_fpp, dataspaces.scalar.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  datasets.step = HDF5Obj(
      H5Dcreate(group.get(), "step", H5T_NATIVE_UINT64, dataspaces.scalar.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
      H5Dclose);
  datasets.ex = HDF5Obj(H5Dcreate(group.get(), "ex", h5_fpp, dataspaces.e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);
  datasets.ey = HDF5Obj(H5Dcreate(group.get(), "ey", h5_fpp, dataspaces.e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);
  datasets.ez = HDF5Obj(H5Dcreate(group.get(), "ez", h5_fpp, dataspaces.e.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);
  datasets.hx = HDF5Obj(H5Dcreate(group.get(), "hx", h5_fpp, dataspaces.h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);
  datasets.hy = HDF5Obj(H5Dcreate(group.get(), "hy", h5_fpp, dataspaces.h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);
  datasets.hz = HDF5Obj(H5Dcreate(group.get(), "hz", h5_fpp, dataspaces.h.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                        H5Dclose);

  SPDLOG_TRACE("exit World::setup_datasets");
}
