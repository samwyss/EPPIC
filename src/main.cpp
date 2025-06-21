#include "scalar.h"
#include "spdlog/spdlog.h"

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

  auto field = Scalar3<int>(2, 2, 2, 4);

  spdlog::debug(field(1, 1, 1));
}
