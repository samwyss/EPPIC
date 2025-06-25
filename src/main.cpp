#include <mdspan/mdspan.hpp>
#include <spdlog/spdlog.h>

#include "scalar.h"
#include "vector.h"
#include "world.h"

/*!
 * main driver function and build target
 * @param argc argument count
 * @param argv argument vector
 * @return
 */
int main(int argc, char **argv) {

  // todo
  // fdtd engine
  // hdf5 io
  // particles
  // configuration
  // parallelization

  spdlog::set_level(spdlog::level::debug);

  const auto fp_precision = "double";

  if (fp_precision == "double") {
    const auto config = Config<double>();
    auto world = World<double>::create(config).value();
    world.advance_to(config.end_time).value();
  } else if (fp_precision == "single") {
    const auto config = Config<float>();
    auto world = World<float>::create(config).value();
    world.advance_to(config.end_time).value();
  } else {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
