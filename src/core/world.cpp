#include "world.h"

World::World(const Config &config)
    : engine(FDTDEngine::create(config).value()), ds_ratio(config.ds_ratio),
      file(H5Fcreate(fmt::format("{}data.h5", config.io_dir.string()).c_str(),
                     H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT),
           H5Fclose),
      io_dir_str(config.io_dir.string()) {}

std::expected<World, std::string> World::create(const Config &config) {
  try {
    return World(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> World::advance_to(const fpp end_t) {
  SPDLOG_TRACE("enter World::advance_to");
  SPDLOG_DEBUG("current time is {:.3e} (s)", time);
  SPDLOG_DEBUG("advance time to {:.3e} (s)", end_t);

  if (end_t > time) {
    // (s) time difference between current state and end time
    const fpp adv_t = end_t - time;

    if (const auto adv_by_result = advance_by(adv_t);
        !adv_by_result.has_value()) {
      SPDLOG_CRITICAL("advance time by advance_by returned with error: {}",
                      adv_by_result.error());
      return std::unexpected(adv_by_result.error());
    }

  } else {
    SPDLOG_WARN(
        "end time of {:.3e} (s) is not greater than current time of {:.3e} (s)",
        end_t, time);
  }

  SPDLOG_TRACE("exit World::advance_to");
  return {};
}

std::expected<void, std::string> World::advance_by(const fpp adv_t) {
  SPDLOG_TRACE("enter World::advance_by");
  SPDLOG_DEBUG("advance time by {:.3e} (s)", adv_t);

  // (s) initial time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  [[maybe_unused]] const auto init_time = time;

  // number of steps required
  const auto steps = engine.calc_num_steps(adv_t);

  // (s) time step
  const fpp dt = adv_t / static_cast<fpp>(steps);
  SPDLOG_DEBUG("timestep: {:.3e} (s)", dt);

  // loop start time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const spdlog::stopwatch watch;

  // todo move me somewhere more logical along with hdf5 file creation maybe
  SimpleXdmf xdmf;
  xdmf.setVersion("3.0");
  xdmf.setNewLineCodeLF();
  xdmf.setIndentSpaceSize(4);
  xdmf.beginDomain();

  // main time loop
  SPDLOG_DEBUG("enter main time loop");
  try {
    for (uint64_t i = 0; i < steps; ++i) {
      SPDLOG_DEBUG("step: {}/{} elapsed time (s): {:.5e}/{:.5e}", i + 1, steps,
                   time, init_time + adv_t);

      // advance by one step
      engine.step(dt);
      time += dt;

      if (0 == i % ds_ratio || i == steps - 1) [[unlikely]] {
        SPDLOG_DEBUG("begin data logging");

        // HDF5 group for this particular step
        const auto group =
            HDF5Obj(H5Gcreate(file.get(), fmt::to_string(i).c_str(),
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                    H5Gclose);

        // write field data
        engine.write_h5(group);

        // todo move me
        xdmf.beginGrid(fmt::format("Step{}", i + 1));

        xdmf.beginTime();
        xdmf.setValue(fmt::to_string(time));
        xdmf.endTime();

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
  xdmf.generate(fmt::format("{}/data.xdmf", io_dir_str));

  // basic loop performance diagnostics
  [[maybe_unused]] const auto loop_time = watch.elapsed().count();
  [[maybe_unused]] const auto num_cells =
      6 * engine.get_field_num_vox() * steps;
  SPDLOG_INFO("loop runtime (s): {:.3e}", loop_time);
  SPDLOG_INFO("voxel compute rate (vox/s): {:.3e}",
              static_cast<double>(num_cells) / loop_time);

  SPDLOG_TRACE("exit FDTDEngine::advance_by");

  return {};
}