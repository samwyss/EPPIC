#include "world.h"

template <std::floating_point T>
World<T>::World(const Config &config)
    : emengine(FDTDEngine<T>::create(config).value()) {}

template <std::floating_point T>
std::expected<World<T>, std::string> World<T>::create(const Config &config) {
  try {
    return World(config);
  } catch (const std::runtime_error &err) {
    return std::unexpected(err.what());
  }
}

template <std::floating_point T>
std::expected<void, std::string> World<T>::advance_to(const T end_t) {
  if (end_t > time) {
    // (s) time difference between current state and end time
    const T adv_t = end_t - time;

    advance_by(adv_t).value();
  }
  return {};
}

template <std::floating_point T>
std::expected<void, std::string> World<T>::advance_by(const T adv_t) {

  // get timesteps from sub solvers

  // loop

  // todo remove me
  emengine.advance_by(adv_t).value();

  return {};
}

// explicit template instantiation
template class World<float>;
template class World<double>;