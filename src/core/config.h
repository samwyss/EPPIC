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

#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <string>
#include <toml11/find.hpp>
#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>

#include "coordinate.h"
#include "type.h"

/*!
 * EPPIC configuration
 */
class Config {
public:
  /*!
   * Config constructor
   * @param input_file_path input file path as std::string
   * @param id unique run identifier
   */
  explicit Config(const std::string &input_file_path, const std::string &id);

  /// (s) end time of simulation
  fpp end_time;

  /// (m) size of bounding box in all directions
  Coord3<fpp> len;

  /// (Hz) maximum frequency to resolve with FDTD engine
  fpp max_frequency;

  /// number of voxels per minimum wavelength for FDTD engine
  size_t num_vox_min_wavelength;

  /// number of voxels per minimum feature dimension for FDTD engine
  size_t num_vox_min_feature;

  /// relative diagonally isotropic permittivity of material inside bounding box
  fpp ep_r;

  /// relative diagonally isotropic permeability of material inside bounding box
  fpp mu_r;

  /// (S / m) diagonally isotropic conductivity of material in bounding box
  fpp sigma;

  /// output directory
  std::filesystem::path out;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio;

private:
  /*!
   * parses and validates [time] section of config
   * @param config toml configuration
   * @return std::expected<void, std::string>
   */
  [[nodiscard]] std::expected<void, std::string> parse_time(const toml::basic_value<toml::type_config> &config);

  /*!
   * parses and validates [geometry] section of config
   * @param config toml configuration
   * @return std::expected<void, std::string>
   */
  [[nodiscard]] std::expected<void, std::string> parse_geometry(const toml::basic_value<toml::type_config> &config);

  /*!
   * parses and validates [material] section of config
   * @param config toml configuration
   * @return std::expected<void, std::string>
   */
  [[nodiscard]] std::expected<void, std::string> parse_material(const toml::basic_value<toml::type_config> &config);

  /*!
   * parses and validates [data] section of config
   * @param config toml configuration
   * @param id unique run identifier
   * @return std::expected<void, std::string>
   */
  [[nodiscard]] std::expected<void, std::string> parse_data(const toml::basic_value<toml::type_config> &config,
                                                            const std::string &id);

  /*!
   * sets up output file structure
   * @param out_dir directory to create filestructure in
   * @param id unique run identifier
   * @return std::expected<std::filesystem::path, std::string>
   */
  [[nodiscard]] static std::expected<std::filesystem::path, std::string>
  setup_dirs(const std::filesystem::path &out_dir, const std::string &id);
};

#endif // CORE_CONFIG_H
