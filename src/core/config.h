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
#include <cxxabi.h>
#include <expected>
#include <filesystem>
#include <fmt/chrono.h>
#include <limits>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <toml11/find.hpp>
#include <toml11/parser.hpp>
#include <toml11/serializer.hpp>
#include <type_traits>
#include <typeinfo>

#include "coordinate.h"
#include "type.h"

/*!
 * possible options for range checking
 * @note single word values imply inclusive/exclusive for both bounds whereas two word values correspond to lower and
 * upper bounds respectively
 */
enum class Bounds { INCL, EXCL, INCL_EXCL, EXCL_INCL };

/*!
 * EPPIC configuration
 */
struct Config {
  /// (s) end time of simulation
  fp_t end_time = 0.0;

  /// (m) size of bounding box in all directions
  Coord3<fp_t> len = {0.0, 0.0, 0.0};

  /// (Hz) maximum frequency to resolve with FDTD engine
  fp_t max_frequency = 0.0;

  /// number of voxels per minimum wavelength for FDTD engine
  ui_t num_vox_min_wavelength = 0;

  /// number of voxels per minimum feature dimension for FDTD engine
  ui_t num_vox_min_feature = 0;

  /// relative diagonally isotropic permittivity of material inside bounding box
  fp_t ep_r = 0.0;

  /// relative diagonally isotropic permeability of material inside bounding box
  fp_t mu_r = 0.0;

  /// (S / m) diagonally isotropic conductivity of material in bounding box
  fp_t sigma = 0.0;

  /// output directory
  std::filesystem::path out = std::filesystem::path("/dev/null");

  /// (s) time between logging events
  /// first and last timestep will always be logged
  fp_t log_period = 0.0;

  /*!
   * initializes configuration from input deck
   * @param input_file_path path to input configuration file
   * @return std::expected<void, std::string> for {success, error} cases respectively
   */
  [[nodiscard]] std::expected<void, std::string> init(const std::string &input_file_path) noexcept;

  [[nodiscard]] std::expected<void, std::string>
  parse_from(const toml::basic_value<toml::type_config> &config) noexcept;

  template <typename T>
  std::expected<T, std::string> parse_item(const toml::basic_value<toml::type_config> &config, const std::string table,
                                           const std::string key) noexcept {
    SPDLOG_TRACE("enter Config::parse");

    const auto type = type_name<T>();

    try {
      const T value = toml::find<T>(config, table, key);
      SPDLOG_DEBUG("`[{}] {}` successfully parsed as {} with value `{}`", table, key, type, value);
      SPDLOG_TRACE("exit Config::parse with success");
      return value;
    } catch (const std::exception &err) {
      const std::string error = fmt::format("parsing `[{}] {}` as {} failed: {}", table, key, type, err.what());
      SPDLOG_CRITICAL(error);
      SPDLOG_TRACE("exit Config::parse with failure");
      return std::unexpected(error);
    }
  }

  /*!
   * returns typename of T as a std::string
   * @tparam T type to get name of
   * @note defaults to mangled typename if demangling fails
   * @return typename of T as a std::string
   */
  template <typename T> std::string type_name() const noexcept {
    SPDLOG_TRACE("enter Config::type_name");

    using TR = std::remove_reference_t<T>;

    int status = 0;
    const std::unique_ptr<char, void (*)(void *)> name_char_arr(
        abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, &status), std::free);

    std::string name = (status == 0) ? name_char_arr.get() : typeid(TR).name();

    if (std::is_const_v<T>) {
      name += " const";
    }

    if (std::is_volatile_v<T>) {
      name += " volatile";
    }

    if (std::is_lvalue_reference_v<T>) {
      name += "&";
    } else if (std::is_rvalue_reference_v<T>) {
      name += "&&";
    }

    SPDLOG_TRACE("exit Config::type_name");
    return name;
  }

  [[nodiscard]] std::expected<void, std::string> validate() noexcept;

  template <typename T>
  static bool in_range(const T value, const T lower, const T upper, const Bounds bounds) noexcept {
    SPDLOG_TRACE("enter Config::in_range");

    bool status = false;

    switch (bounds) {
    case Bounds::INCL:
      status = value >= lower && value <= upper;
      SPDLOG_DEBUG("value `{}` within range [{}, {}]: {}", value, lower, upper, status);
      break;
    case Bounds::EXCL:
      status = value > lower && value < upper;
      SPDLOG_DEBUG("value `{}` within range ({}, {}): {}", value, lower, upper, status);
      break;
    case Bounds::INCL_EXCL:
      status = value >= lower && value < upper;
      SPDLOG_DEBUG("value `{}` within range [{}, {}): {}", value, lower, upper, status);
      break;
    case Bounds::EXCL_INCL:
      status = value > lower && value <= upper;
      SPDLOG_DEBUG("value `{}` within range ({}, {}]: {}", value, lower, upper, status);
      break;
    }

    SPDLOG_TRACE("exit Config::in_range");
    return status;
  }

  std::expected<std::filesystem::path, std::string> setup_out(const std::string &id) const noexcept;
};

#endif // CORE_CONFIG_H
