#include "world.h"

World::World()
    : config(Config()), emengine(FDTDEngine::create(config).value()) {}

std::expected<World, std::string> World::create() {
  try {
    return World();
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> World::advance_to(const double end_t) {
  if (end_t > time) {
    // (s) time difference between current state and end time
    const double adv_t = end_t - time;

    advance_by(adv_t).value();
  }
  return {};
}

std::expected<void, std::string> World::advance_by(const double adv_t) {

  // get timesteps from sub solvers

  // loop

  // todo remove me
  emengine.advance_by(adv_t).value();

  return {};
}
