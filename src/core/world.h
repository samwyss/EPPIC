/*
 * Copyright (C) 2025 Samuel Wyss
 *
 * This file is part of EPPIC.
 *
 * EPPIC is free software: you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * EPPIC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EPPIC. If not, see
 * <https://www.gnu.org/licenses/>.
 */

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
struct World {
  /// configuration from file
  const Config cfg;

  /// output HDF5 file
  const HDF5Obj h5;

  /// (s) elapsed time
  fp_t time = 0.0;

  /// (F/m) diagonally isotropic permittivity of material inside bounding box
  const fp_t ep;

  /// (H/m) diagonally isotropic permeability of material inside bounding box
  const fp_t mu;

  /// number of voxels in magnetic field
  const Coord3<ui_t> nv_h;

  /// number of voxels in electric field
  const Coord3<ui_t> nv_e;

  /// (m) spatial increments in all directions
  const Coord3<fp_t> d;

  /// (m) inverse spatial increments in all directions
  const Coord3<fp_t> d_inv;

  /// (V/m) electric field vector
  /// NOTE: as configured e wraps h to make it easier to manage boundary conditions
  const Vector3<fp_t> e;

  /// (A/m) magnetic field vector
  const Vector3<fp_t> h;

  /// dataspaces for writable data
  Dataspaces dataspaces;

  /// datasets for writable data
  Datasets datasets;

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
  [[nodiscard]] std::expected<void, std::string> advance_to(fp_t end_t);

  /*!
   * advances internal state by a given time period
   * @param adv_t (s) time period to advance by
   * @return void
   */
  [[nodiscard]] std::expected<void, std::string> advance_by(fp_t adv_t);

  /*!
   * returns configured output directory
   *
   * useful for setting a logging directory
   * @return std::filesystem::path
   */
  [[nodiscard]] std::filesystem::path get_output_dir() const;

  /*!
   * initializes h5
   * @return HDF5Obj
   */
  [[nodiscard]] HDF5Obj init_h5() const;

  /*!
   * initializes nv_h
   * @return Coord3<ui_t>
   */
  [[nodiscard]] Coord3<ui_t> init_nv_h() const;

  /*!
   * initializes nv_e
   * @return Coord3<ui_t>
   */
  [[nodiscard]] Coord3<ui_t> init_nv_e() const;

  /*!
   * initializes d
   * @return Coord3<fp_t>
   */
  [[nodiscard]] Coord3<fp_t> init_d() const;

  /*!
   * initializes d_inv
   * @return Coord3<fp_t>
   */
  [[nodiscard]] Coord3<fp_t> init_d_inv() const;

  /*!
   * calculates the number of steps required to advance engine state by some
   * time period
   * @param adv_t (s) time period to advance by
   * @return ui_t
   */
  [[nodiscard]] ui_t calc_num_steps(fp_t adv_t) const;

  /*!
   * calculates the number of steps required to model a given time span
   * @param time_span (s) time span to be modeled
   * @return number of steps required by CFL stability condition
   */
  [[nodiscard]] ui_t calc_cfl_steps(fp_t time_span) const;

  /*!
   * advances internal field state by one time step
   * @param dt (s) time step
   * @return void
   */
  void step(fp_t dt);

  /*!
   * advances internal electric field state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_e(fp_t ea, fp_t eb) const;

  /*!
   * advances internal magnetic field state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_h(fp_t hxa, fp_t hya, fp_t hza) const;

  /*!
   * advances internal electric field x-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ex(fp_t ea, fp_t eb) const;
  /*!
   * advances internal electric field y-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ey(fp_t ea, fp_t eb) const;

  /*!
   * advances internal electric field z-component state by one time step
   * @param ea electric field a loop constant
   * @param eb electric field b loop constant
   */
  void update_ez(fp_t ea, fp_t eb) const;

  /*!
   * advances internal magnetic field x-component state by one time step
   * @param hya magnetic field a loop constant for y-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hx(fp_t hya, fp_t hza) const;

  /*!
   * advances internal magnetic field y-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hza magnetic field a loop constant for z-component
   */
  void update_hy(fp_t hxa, fp_t hza) const;

  /*!
   * advances internal magnetic field z-component state by one time step
   * @param hxa magnetic field a loop constant for x-component
   * @param hya magnetic field a loop constant for y-component
   */
  void update_hz(fp_t hxa, fp_t hya) const;

  /*!
   * logs runtime data to out
   *
   * todo improve error handling
   *
   * @param hyperslab hyperslab index to write to
   * @param step current time step
   */
  void log(ui_t hyperslab, ui_t step) const;

  /*!
   * logs metadata required for gen_xdmf.py
   *
   * todo improve error handling
   *
   * @param group HDF5 group to write to
   * @param dt (s) timestep
   * @param num number of logged steps
   */
  void log_metadata(const HDF5Obj &group, double dt, ui_t num) const;

  /*!
   * sets up dataspaces for logging
   *
   * todo improve error handling
   *
   * @param num number of logged steps
   */
  void setup_dataspaces(ui_t num);

  /*!
   * sets up datasets for logging
   *
   * todo improve error handling
   *
   * @param group group to create datasets within
   */
  void setup_datasets(const HDF5Obj &group);
};

#endif // CORE_WORLD_H
