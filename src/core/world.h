#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <string>

#include "config.h"
#include "io.h"
#include "numeric.h"
#include "physical.h"
#include "vector.h"

/*!
 * EPPIC World object
 */
class World {
public:
  /*!
   * World constructor
   * @param input_file_path input file path as std::string
   * @param id unique run identifier
   */
  World(const std::string &input_file_path, const std::string &id);

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
   * initializes h5
   * @return HDF5Obj
   */
  [[nodiscard]] HDF5Obj init_h5() const;

  /*!
   * initializes nv_h
   * @return Coord3<size_t>
   */
  [[nodiscard]] Coord3<size_t> init_nv_h() const;

  /*!
   * initializes nv_e
   * @return Coord3<size_t>
   */
  [[nodiscard]] Coord3<size_t> init_nv_e() const;

  /*!
   * initializes d
   * @return Coord3<fpp>
   */
  [[nodiscard]] Coord3<fpp> init_d() const;

  /*!
   * initializes d_inv
   * @return Coord3<fpp>
   */
  [[nodiscard]] Coord3<fpp> init_d_inv() const;

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
   * logs runtime data to out
   *
   * todo improve error handling
   *
   * @param hyperslab hyperslab index to write to
   */
  void log(uint64_t hyperslab) const;

  /*!
   * writes metadata required for gen_xdmf.py
   *
   * todo improve error handling
   *
   * @param group HDF5 group to write to
   * @param dt (s) timestep
   * @param num number of logged steps
   */
  void write_metadata(const HDF5Obj &group, double dt, uint64_t num) const;

  /*!
   * sets up dataspaces for logging
   *
   * todo improve error handling
   *
   * @param num number of logged steps
   */
  void setup_dataspaces(uint64_t num);

  /*!
   * sets up datasets for logging
   *
   * todo improve error handling
   *
   * @param group group to create datasets within
   */
  void setup_datasets(const HDF5Obj &group);

  /// configuration from file
  const Config cfg;

  /// output HDF5 file
  const HDF5Obj h5;

  /// output HDF5 floating point type
  const hid_t h5_fpp = (std::is_same_v<fpp, double>) ? H5T_NATIVE_DOUBLE : H5T_NATIVE_FLOAT;

  /// (s) elapsed time
  fpp time = 0.0;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const fpp ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const fpp mu;

  /// number of voxels in electric field
  const Coord3<size_t> nv_e;

  /// number of voxels in magnetic field
  const Coord3<size_t> nv_h;

  /// (m) spatial increments in all directions
  const Coord3<fpp> d;

  /// (m) inverse spatial increments in all directions
  const Coord3<fpp> d_inv;

  /// (V/m) electric field vector
  /// NOTE: as configured e wraps h to make it easier to manage boundary conditions
  const Vector3<fpp> e;

  /// (A/m) magnetic field vector
  const Vector3<fpp> h;

  /// dataspaces for writable data
  Dataspaces dataspaces;

  /// datasets for writable data
  Datasets datasets;
};

#endif // CORE_WORLD_H
