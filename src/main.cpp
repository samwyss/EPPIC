#include <mdspan/mdspan.hpp>
#include <spdlog/spdlog.h>

#include "scalar.h"
#include "vector.h"

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

  auto field = Scalar3<double>({3, 3, 3}, 5.0);
  auto vfield = Vector3<double>({10, 10, 10}, 5.0);

  for (size_t i = 0; i < field.data.extent(0); ++i) {
    for (size_t j = 0; j < field.data.extent(1); ++j) {
      for (size_t k = 0; k < field.data.extent(2); ++k) {
        spdlog::debug(field.data[4, 4, 4]);
      }
    }
  }
}
