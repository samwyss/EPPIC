#include "emengine.h"

EMEngine::EMEngine(const Config &config)
    : e(Vector3<double>({1, 1, 1}, 0.0)), h(Vector3<double>({1, 1, 1}, 0.0)) {}

std::expected<EMEngine, std::string> EMEngine::create(const Config &config) {
  try {
    return std::expected<EMEngine, std::string>{std::in_place, config};
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}
