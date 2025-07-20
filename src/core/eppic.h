#ifndef CORE_EPPIC_H
#define CORE_EPPIC_H

#include <expected>
#include <simple_xdmf.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "config.h"
#include "hdf5_wrapper.h"

/*!
 * World object
 */
class World {
public:
  /*!
   * World static factory method
   * @param input_file_path todo document
   * @param id todo document
   * @return void
   */
  [[nodiscard]] static std::expected<World, std::string> create(const std::string &input_file_path,
                                                                const std::string &id);

  /*!
   * advances internal state to an end time
   *
   * will do nothing in the event that end_t <= time
   * @param end_t (s) end time
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_to(fpp end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_by(fpp adv_t);

private:
  /*!
   * World constructor
   * @param input_file_path todo document
   * @param id todo document
   */
  explicit World(const std::string &input_file_path, const std::string &id);

  /// configuration from file
  const Config cfg;

  /// output HDF5 file
  const HDF5Obj h5;

  /// xdmf writer
  SimpleXdmf xdmf;

  /// (s) elapsed time
  fpp time = 0.0;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const fpp ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const fpp mu;

  /// number of voxels in all directions
  Coord3<size_t> nv{};

  /// (m) spatial increments in all directions
  Coord3<fpp> d;

  /// (m) inverse spatial increments in all directions
  Coord3<fpp> d_inv;

  /// (V/m) electric field vector
  Vector3<fpp> e;

  /// (A/m) magnetic field vector
  Vector3<fpp> h;
};

#endif // CORE_EPPIC_H
