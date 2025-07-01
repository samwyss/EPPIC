#include "world.h"

template <std::floating_point T>
World<T>::World(const Config<T> &config)
    : engine(FDTDEngine<T>::create(config).value()) {}

template <std::floating_point T>
std::expected<World<T>, std::string> World<T>::create(const Config<T> &config) {
  try {
    return World(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
std::expected<void, std::string> World<T>::advance_to(const T end_t) {
  SPDLOG_TRACE("enter World<T>::advance_to");
  SPDLOG_DEBUG("current time is {:.3e} (s)", time);
  SPDLOG_DEBUG("advance time to {:.3e} (s)", end_t);

  if (end_t > time) {
    // (s) time difference between current state and end time
    const T adv_t = end_t - time;

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

  SPDLOG_TRACE("exit World<T>::advance_to");
  return {};
}

template <std::floating_point T>
std::expected<void, std::string> World<T>::advance_by(const T adv_t) {
  SPDLOG_TRACE("enter World<T>::advance_by");
  SPDLOG_DEBUG("advance time by {:.3e} (s)", adv_t);

  // (s) initial time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
  [[maybe_unused]] const auto init_time = time;

  // number of steps required
  const auto steps = engine.calc_num_steps(adv_t);

  // (s) time step
  const T dt = adv_t / static_cast<T>(steps);
  SPDLOG_DEBUG("timestep: {:.3e} (s)", dt);

  // loop start time
  // NOTE only used if SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
  [[maybe_unused]] const spdlog::stopwatch watch;

  // main time loop
  SPDLOG_DEBUG("enter main time loop");
  try {
    for (uint64_t i = 0; i < steps; ++i) {

      // advance by one step
      engine.step(dt);
      time += dt;

      SPDLOG_TRACE("step: {}/{} elapsed time: {:.5e}/{:.5e} (s)", i + 1, steps,
                   time, init_time + adv_t);
    }
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("main time loop returned with error: {}", err.what());
    return std::unexpected(err.what());
  }
  SPDLOG_DEBUG("exit main time loop with success");

  // basic loop performance diagnostics
  [[maybe_unused]] const auto loop_time = watch.elapsed().count();
  [[maybe_unused]] const auto num_cells =
      6 * engine.get_field_num_vox() * steps;
  SPDLOG_INFO("loop runtime: {:.3e} (s)", loop_time);
  SPDLOG_INFO("voxel compute rate {:.3e}",
              static_cast<double>(num_cells) / loop_time);

  SPDLOG_TRACE("exit FDTDEngine<T>::advance_by");

  return {};
}

// explicit template instantiation
template class World<float>;
template class World<double>;