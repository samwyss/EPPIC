#include "world.h"

World::World() { throw std::runtime_error("World::World()"); }

std::expected<World, std::string> World::create() {
  try {
    return World();
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> World::adv_by() { return {}; }

std::expected<void, std::string> World::adv_to() { return {}; }
