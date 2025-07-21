#include "world.h"

World::World(const std::string &input_file_path, const std::string &id)
    : cfg(input_file_path, id), ep(cfg.ep_r * VAC_PERMITTIVITY), mu(cfg.mu_r * VAC_PERMEABILITY) {
  h5 = HDF5Obj(H5Fcreate((cfg.out / "data.h5").c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT), H5Fclose);
  SPDLOG_DEBUG("created output HDF5 file at `{}`", (cfg.out / "data.h5").string());

  xdmf.setVersion("3.0");
  xdmf.setNewLineCodeLF();
  xdmf.setIndentSpaceSize(4);
  SPDLOG_DEBUG("initialized XDMF writer");

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

  // number of voxels in each direction snapped to ds
  nv = {static_cast<size_t>(ceil(static_cast<double>(cfg.len.x) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(cfg.len.y) / ds)),
        static_cast<size_t>(ceil(static_cast<double>(cfg.len.z) / ds))};
  SPDLOG_DEBUG("field voxel dimensions: {} x {} x {}", nv.x, nv.y, nv.z);
  SPDLOG_DEBUG("voxels to update each step: {}", 6 * nv.x);

  // (m) final spatial steps
  d = {cfg.len.x / static_cast<fpp>(nv.x), cfg.len.y / static_cast<fpp>(nv.y), cfg.len.z / static_cast<fpp>(nv.z)};
  SPDLOG_DEBUG("voxel size (m): {:.3e} x {:.3e} x {:.3e}", d.x, d.y, d.z);

  // (m^-1) inverse spatial steps
  d_inv = {static_cast<fpp>(1.0) / d.x, static_cast<fpp>(1.0) / d.y, static_cast<fpp>(1.0) / d.z};
  SPDLOG_DEBUG("inverse voxel size (m^-1) , {:.3e} x {:.3e} x {:.3e}", d_inv.x, d_inv.y, d_inv.z);

  // initialize fields
  e = Vector3<fpp>(nv, 0.0);
  h = Vector3<fpp>(nv, 0.0);
}

std::expected<World, std::string> World::create(const std::string &input_file_path, const std::string &id) {
  SPDLOG_TRACE("enter World::create");
  try {
    auto world = World(input_file_path, id);
    SPDLOG_TRACE("exit World::create with success");
    return world;
  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("failed to configure World object: {}", err.what());
    return std::unexpected(err.what());
  }
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
  SPDLOG_DEBUG("current time is {:.3e} (s)", time);
  SPDLOG_DEBUG("advance time to {:.3e} (s)", end_t);

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
  SPDLOG_DEBUG("advance time by {:.3e} (s)", adv_t);

  // (s) initial time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  [[maybe_unused]] const auto init_time = time;

  // number of steps required
  const auto steps = calc_num_steps(adv_t);

  // (s) time step
  const fpp dt = adv_t / static_cast<fpp>(steps);
  SPDLOG_DEBUG("timestep: {:.3e} (s)", dt);

  // loop start time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto start_time = std::chrono::high_resolution_clock::now();
  ;

  xdmf.beginDomain();

  // main time loop
  SPDLOG_DEBUG("enter main time loop");
  try {
    for (uint64_t i = 0; i < steps; ++i) {
      SPDLOG_DEBUG("step: {}/{} elapsed time (s): {:.5e}/{:.5e}", i + 1, steps, time, init_time + adv_t);

      // advance by one step
      step(dt);
      time += dt;

      if (0 == i % cfg.ds_ratio || i == steps - 1) [[unlikely]] {
        SPDLOG_DEBUG("begin data logging");

        // HDF5 group for this particular step
        const auto group =
            HDF5Obj(H5Gcreate(h5.get(), fmt::to_string(i).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose);

        // write field data
        write_h5(group);

        // todo move me
        xdmf.beginGrid(fmt::format("Step{}", i + 1));

        xdmf.beginTime();
        xdmf.setValue(fmt::to_string(time));
        xdmf.endTime();

        // todo the meshes and otherthings are very wrong, look into vector
        // fields as well
        xdmf.beginStructuredTopology("Topo1", "3DCoRectMesh");
        xdmf.setDimensions("5 5 5"); // number of points not cells
        xdmf.endStructuredTopology();
        xdmf.beginGeometory("FieldMesh", "ORIGIN_DXDYDZ");
        xdmf.beginDataItem();
        xdmf.setDimensions("3");
        xdmf.setFormat("XML");
        xdmf.addItem("0 0 0");
        xdmf.endDataItem();
        xdmf.beginDataItem();
        xdmf.setDimensions("3");
        xdmf.setFormat("XML");
        xdmf.addItem("1 1 1");
        xdmf.endDataItem();
        xdmf.endGeometory();

        xdmf.beginAttribute("ex");
        xdmf.setCenter("Cell");
        xdmf.beginDataItem();
        xdmf.setDimensions("4 4 4");
        xdmf.setPrecision("4");
        xdmf.setFormat("HDF");
        xdmf.addItem(fmt::format("data.h5:/{}/ex", i));
        xdmf.endDataItem();
        xdmf.endAttribute();

        xdmf.endGrid();

        SPDLOG_DEBUG("end data logging");
      }
    }
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("main time loop returned with error: {}", err.what());
    return std::unexpected(err.what());
  }
  SPDLOG_DEBUG("exit main time loop with success");

  // todo do something with me
  xdmf.endDomain();
  // xdmf.generate(fmt::format("{}/data.xdmf", io_dir_str)); todo uncomment me

  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto end_time = std::chrono::high_resolution_clock::now();
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto num_cells = 6 * nv.x * steps; // assumes number of voxels in all fields is the same as x
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto loop_time = end_time - start_time;
  SPDLOG_INFO("loop runtime: {}", loop_time);
  SPDLOG_INFO("voxel compute rate (vox/s): {:.3e}",
              static_cast<double>(num_cells) / std::chrono::duration_cast<std::chrono::seconds>(loop_time).count());

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

  // todo remove branches
  for (size_t i = 0; i < h.x.extent(0); ++i) {
    for (size_t j = 0; j < h.x.extent(1); ++j) {
      for (size_t k = 0; k < h.x.extent(2); ++k) {
        [[likely]] if (j != h.x.extent(1) - 1 && k != h.x.extent(2) - 1) {
          // j- k-low volume
          h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) + hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
        } else if (j != h.x.extent(1) - 1 && k == h.x.extent(2) - 1) {
          // k-high plane
          h.x[i, j, k] += -hya * (e.z[i, j + 1, k] - e.z[i, j, k]) + hza * (static_cast<fpp>(0.0) - e.y[i, j, k]);
        } else if (j == h.x.extent(1) - 1 && k != h.x.extent(2) - 1) {
          // j-high plane
          h.x[i, j, k] += -hya * (static_cast<fpp>(0.0) - e.z[i, j, k]) + hza * (e.y[i, j, k + 1] - e.y[i, j, k]);
        } else {
          // j- k-high line
          h.x[i, j, k] += -hya * (static_cast<fpp>(0.0) - e.z[i, j, k]) + hza * (static_cast<fpp>(0.0) - e.y[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit World::update_hx");
}

void World::update_hy(const fpp hxa, const fpp hza) const {
  SPDLOG_TRACE("enter World::update_hy");

  // todo remove branches
  for (size_t i = 0; i < h.y.extent(0); ++i) {
    for (size_t j = 0; j < h.y.extent(1); ++j) {
      for (size_t k = 0; k < h.y.extent(2); ++k) {
        [[likely]] if (i != h.y.extent(0) - 1 && k != h.y.extent(2) - 1) {
          // i- k-low volume
          h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) + hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
        } else if (i != h.y.extent(0) - 1 && k == h.y.extent(2) - 1) {
          // k-high plane
          h.y[i, j, k] += -hza * (static_cast<fpp>(0.0) - e.x[i, j, k]) + hxa * (e.z[i + 1, j, k] - e.z[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && k != h.y.extent(2) - 1) {
          // i-high plane
          h.y[i, j, k] += -hza * (e.x[i, j, k + 1] - e.x[i, j, k]) + hxa * (static_cast<fpp>(0.0) - e.z[i, j, k]);
        } else {
          // i- k-high line
          h.y[i, j, k] += -hza * (static_cast<fpp>(0.0) - e.x[i, j, k]) + hxa * (static_cast<fpp>(0.0) - e.z[i, j, k]);
        }
      }
    }
  }

  SPDLOG_TRACE("exit World::update_hy");
}

void World::update_hz(const fpp hxa, const fpp hya) const {
  SPDLOG_TRACE("enter World::update_hz");

  // todo remove branches
  for (size_t i = 0; i < h.z.extent(0); ++i) {
    for (size_t j = 0; j < h.z.extent(1); ++j) {
      for (size_t k = 0; k < h.z.extent(2); ++k) {
        [[likely]] if (i != h.y.extent(0) - 1 && j != h.y.extent(1) - 1) {
          // i- j-low volume
          h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) + hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
        } else if (i != h.y.extent(0) - 1 && j == h.y.extent(1) - 1) {
          // j-high plane
          h.z[i, j, k] += -hxa * (e.y[i + 1, j, k] - e.y[i, j, k]) + hya * (static_cast<fpp>(0.0) - e.x[i, j, k]);
        } else if (i == h.y.extent(0) - 1 && j != h.y.extent(1) - 1) {
          // i-high plane
          h.z[i, j, k] += -hxa * (static_cast<fpp>(0.0) - e.y[i, j, k]) + hya * (e.x[i, j + 1, k] - e.x[i, j, k]);
        } else {
          // i- j-high line
          h.z[i, j, k] += -hxa * (static_cast<fpp>(0.0) - e.y[i, j, k]) + hya * (static_cast<fpp>(0.0) - e.x[i, j, k]);
        }
      }
    }
  }
  SPDLOG_TRACE("exit World::update_hz");
}

void World::write_h5(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5");

  write_h5_e(group);
  write_h5_h(group);

  SPDLOG_TRACE("exit World::write_h5");
}

void World::write_h5_e(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_e");

  write_h5_ex(group);
  write_h5_ey(group);
  write_h5_ez(group);
  SPDLOG_TRACE("exit World::write_h5_e");
}

void World::write_h5_h(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_h");

  write_h5_hx(group);
  write_h5_hy(group);
  write_h5_hz(group);
  SPDLOG_TRACE("exit World::write_h5_h");
}

void World::write_h5_ex(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_ex");

  // field dimensions
  const hsize_t dims[3] = {e.x.extent(0), e.x.extent(1), e.x.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ex", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.x.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ex", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.x.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_ex");
}

void World::write_h5_ey(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_ey");

  // field dimensions
  const hsize_t dims[3] = {e.y.extent(0), e.y.extent(1), e.y.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ey", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.y.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ey", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.y.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_ey");
}

void World::write_h5_ez(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_ez");

  // field dimensions
  const hsize_t dims[3] = {e.z.extent(0), e.z.extent(1), e.z.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ez", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.z.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "ez", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, e.z.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_ez");
}

void World::write_h5_hx(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_hx");

  // field dimensions
  const hsize_t dims[3] = {h.x.extent(0), h.x.extent(1), h.x.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hx", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.x.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hx", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.x.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_hx");
}

void World::write_h5_hy(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_hy");

  // field dimensions
  const hsize_t dims[3] = {h.y.extent(0), h.y.extent(1), h.y.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hy", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.y.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hy", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.y.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_hy");
}

void World::write_h5_hz(const HDF5Obj &group) const {
  SPDLOG_TRACE("enter World::write_h5_hz");

  // field dimensions
  const hsize_t dims[3] = {h.z.extent(0), h.z.extent(1), h.z.extent(2)};

  // HDF5 dataspace
  const auto dspace = HDF5Obj(H5Screate_simple(3, dims, nullptr), H5Sclose);

  // compile time switch on fpp
  if constexpr (std::is_same_v<fpp, double>) {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hz", H5T_NATIVE_DOUBLE, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.z.data_handle());

  } else {
    // HDF5 dataset
    const auto dset = HDF5Obj(
        H5Dcreate(group.get(), "hz", H5T_NATIVE_FLOAT, dspace.get(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);

    // write field data
    H5Dwrite(dset.get(), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, h.z.data_handle());
  }

  SPDLOG_TRACE("exit World::write_h5_hz");
}
