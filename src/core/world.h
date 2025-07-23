#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <fmt/chrono.h>
#include <simple_xdmf.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "config.h"
#include "hdf5_wrapper.h"
#include "numeric.h"
#include "physical.h"
#include "vector.h"

/*!
 * electromagnetic field enum
 */
enum class EMField { E, H };

/*!
 * EPPIC World object
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
   * advances internal state to `end_time` parameter as defined in configuration file
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> run();

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

  /*!
   * returns configured output directory
   *
   * useful for setting a logging directory
   * @return std::filesystem::path
   */
  [[nodiscard]] std::filesystem::path get_output_dir() const;

private:
  /*!
   * World constructor
   * @param input_file_path todo document
   * @param id todo document
   */
  explicit World(const std::string &input_file_path, const std::string &id);

  /*!
   * calculates the number of steps required to advance engine state by some
   * time period
   * @param adv_t (s) time period to advance by
   * @return uint64_t
   */
  [[nodiscard]] uint64_t calc_num_steps(fpp adv_t) const;

  /*!
   * calculates the number of steps required to model a given time span
   * @param time_span (s) time span to be modeled
   * @return number of steps required by CFL stability condition
   */
  [[nodiscard]] uint64_t calc_cfl_steps(fpp time_span) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  void step(fpp dt);

  /*!
   * advances internal electric field state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_e(fpp ea, fpp eb) const;

  /*!
   * advances internal magnetic field state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_h(fpp hxa, fpp hya, fpp hza) const;

  /*!
   * advances internal electric field x-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ex(fpp ea, fpp eb) const;
  /*!
   * advances internal electric field y-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ey(fpp ea, fpp eb) const;

  /*!
   * advances internal electric field z-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ez(fpp ea, fpp eb) const;

  /*!
   * advances internal magnetic field x-component state by one time step
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hx(fpp hya, fpp hza) const;

  /*!
   * advances internal magnetic field y-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hy(fpp hxa, fpp hza) const;

  /*!
   * advances internal magnetic field z-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   */
  void update_hz(fpp hxa, fpp hya) const;

  /*!
   * writes an electromagnetic field as Vector3 to HDF5 group
   * @param group HDF5 group to write to
   * @param field field to be written
   * @param type electromagnetic field type
   *
   * this is not error handled as if writing to HDF5 fails no exception is thrown
   * todo improve error handling
   *
   * @return void
   */
  void h5_write_field(const HDF5Obj &group, const Vector3<fpp> &field, EMField type);

  /// configuration from file
  const Config cfg;

  /// output HDF5 file
  HDF5Obj h5;

  /// xdmf writer
  SimpleXdmf xdmf;

  /// (s) elapsed time
  fpp time = 0.0;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const fpp ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const fpp mu;

  /// number of voxels in all directions
  Coord3<size_t> nv;

  /// (m) spatial increments in all directions
  Coord3<fpp> d;

  /// (m) inverse spatial increments in all directions
  Coord3<fpp> d_inv;

  /// (V/m) electric field vector
  Vector3<fpp> e;

  /// (A/m) magnetic field vector
  Vector3<fpp> h;
};

#endif // CORE_WORLD_H
