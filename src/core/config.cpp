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

#include "config.h"

#include <expected>

std::expected<void, std::string> Config::init(const std::string &input_file_path) noexcept {
  SPDLOG_TRACE("enter Config::init");

  std::filesystem::path input_file;
  try {
    input_file = std::filesystem::canonical(input_file_path);
  } catch (const std::exception &err) {
    const std::string error = fmt::format(
        "input file path `{}` could not be canonicalized: {} ... please ensure this is a valid path and rerun",
        input_file.string(), err.what());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  if (!std::filesystem::is_regular_file(input_file)) {
    SPDLOG_CRITICAL("input file path `{}` is not a regular file", input_file_path);

    if (!std::filesystem::is_directory(input_file)) {
      const std::string error = fmt::format(
          "input file path `{}` is a directory not a file ... please correct and rerun", input_file.string());
      SPDLOG_CRITICAL(error);
      return std::unexpected(error);
    }

    const std::string error =
        fmt::format("input file path `{}` is not regular file ... please correct and rerun", input_file.string());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("input file path `{}` successfully verified", input_file.string());

  if (const auto parse_result = toml::try_parse(input_file); parse_result.is_ok()) {
    SPDLOG_DEBUG("input file `{}` is valid toml", input_file.string());

    const auto &config = parse_result.unwrap();

    if (const auto result = parse_from_toml(config); !result.has_value()) {
      const std::string error = fmt::format("failed to parse configuration file: {}", result.error());
      SPDLOG_CRITICAL(error);
      return std::unexpected(error);
    }

  } else {
    const std::string error =
        fmt::format("failed to parse config file `{}` ... provided file contains invalid toml", input_file.string());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  if (const auto result = validate(); !result.has_value()) {
    const std::string error = fmt::format("failed to validate initial state: {}", result.error());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  SPDLOG_INFO("configuration summary");
  SPDLOG_INFO("end time (s): {:.3e}", end_time);
  SPDLOG_INFO("bounding box (m): {:.3e} x {:.3e} x {:.3e}", len.x, len.y, len.z);
  SPDLOG_INFO("maximum frequency to resolve (Hz): {:.3e}", max_frequency);
  SPDLOG_INFO("number of voxels to resolve minimum wavelength: {}", num_vox_min_wavelength);
  SPDLOG_INFO("number of voxels to resolve minimum feature size: {}", num_vox_min_feature);
  SPDLOG_INFO("bounding box relative permittivity: {:.3e}", ep_r);
  SPDLOG_INFO("bounding box relative permeability: {:.3e}", mu_r);
  SPDLOG_INFO("bounding box conductivity (S / m): {:.3e}", sigma);
  SPDLOG_INFO("path to store output data: {}", out.string());
  SPDLOG_INFO("period between logging steps {:.3e}", log_period);

  SPDLOG_TRACE("exit Config::init");
  return {};
}

std::expected<void, std::string> Config::parse_from_toml(const toml::basic_value<toml::type_config> &config) noexcept {
  SPDLOG_TRACE("enter Config::parse_from");

  if (auto result = parse_item<fp_t>(config, "time", "end_time"); result.has_value()) {
    end_time = result.value();
  } else {
    return std::unexpected(result.error());
  }

  fp_t x_len = 0;
  if (auto result = parse_item<fp_t>(config, "geometry", "x_len"); result.has_value()) {
    x_len = result.value();
  } else {
    return std::unexpected(result.error());
  }

  fp_t y_len = 0;
  if (auto result = parse_item<fp_t>(config, "geometry", "y_len"); result.has_value()) {
    y_len = result.value();
  } else {
    return std::unexpected(result.error());
  }

  fp_t z_len = 0;
  if (auto result = parse_item<fp_t>(config, "geometry", "z_len"); result.has_value()) {
    z_len = result.value();
  } else {
    return std::unexpected(result.error());
  }

  len = {x_len, y_len, z_len};

  if (auto result = parse_item<fp_t>(config, "geometry", "max_frequency"); result.has_value()) {
    max_frequency = result.value();
  } else {
    return std::unexpected(result.error());
  }

  if (auto result = parse_item<ui_t>(config, "geometry", "num_vox_min_wavelength"); result.has_value()) {
    num_vox_min_wavelength = result.value();
  } else {
    return std::unexpected(result.error());
  }

  if (auto result = parse_item<ui_t>(config, "geometry", "num_vox_min_feature"); result.has_value()) {
    num_vox_min_feature = result.value();
  } else {
    return std::unexpected(result.error());
  }

  if (auto result = parse_item<fp_t>(config, "geometry", "ep_r"); result.has_value()) {
    ep_r = result.value();
  } else {
    return std::unexpected(result.error());
  }

  if (auto result = parse_item<fp_t>(config, "geometry", "mu_r"); result.has_value()) {
    mu_r = result.value();
  } else {
    return std::unexpected(result.error());
  }

  if (auto result = parse_item<fp_t>(config, "geometry", "sigma"); result.has_value()) {
    sigma = result.value();
  } else {
    return std::unexpected(result.error());
  }

  std::string out_dir_str;
  if (auto result = parse_item<std::string>(config, "data", "out_dir"); result.has_value()) {
    out_dir_str = result.value();
  } else {
    return std::unexpected(result.error());
  }
  out = std::filesystem::path(out_dir_str);

  if (auto result = parse_item<fp_t>(config, "data", "log_period"); result.has_value()) {
    log_period = result.value();
  } else {
    return std::unexpected(result.error());
  }

  SPDLOG_TRACE("exit Config::parse_from");
  return {};
}

std::expected<void, std::string> Config::validate() noexcept {
  SPDLOG_TRACE("enter Config::validate");

  if (!in_range(end_time, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`end_time` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`end_time` passed all checks");

  if (!in_range(len.x, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`x_len` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`x_len` passed all checks");

  if (!in_range(len.y, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`y_len` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`y_len` passed all checks");

  if (!in_range(len.z, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`z_len` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`z_len` passed all checks");

  if (!in_range(max_frequency, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`max_frequency` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`max_frequency` passed all checks");

  if (!in_range(num_vox_min_wavelength, static_cast<ui_t>(0), std::numeric_limits<ui_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error =
        fmt::format("`num_vox_min_wavelength` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`num_vox_min_wavelength` passed all checks");

  if (!in_range(num_vox_min_feature, static_cast<ui_t>(0), std::numeric_limits<ui_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error =
        fmt::format("`num_vox_min_feature` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`num_vox_min_feature` passed all checks");

  if (!in_range(ep_r, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`ep_r` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`ep_r` passed all checks");

  if (!in_range(mu_r, 0.0, std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`mu_r` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`mu_r` passed all checks");

  if (!in_range(sigma, 0.0, std::numeric_limits<fp_t>::max(), Bounds::INCL)) {
    const std::string error = fmt::format("`sigma` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`sigma` passed all checks");

  const auto out_str = out.string();
  try {
    out = std::filesystem::canonical(out);
    SPDLOG_DEBUG("`out_dir` successfully canonicalized with value`{}`", out.string());
  } catch (const std::filesystem::filesystem_error &err) {
    const std::string error =
        fmt::format("unable to canonicalize `out_dir` with path `{}`: {} ... please ensure you are using either an "
                    "absolute path or the appropriate relative path from your current working directory",
                    out.string(), err.what());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  if (!std::filesystem::is_directory(out)) {
    const std::string error =
        fmt::format("path `out_dir` with path `{}`: is not a directory on this filesystem ... please correct and rerun",
                    out.string());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  if (!in_range(log_period, static_cast<fp_t>(0), std::numeric_limits<fp_t>::max(), Bounds::EXCL_INCL)) {
    const std::string error = fmt::format("`log_period` is not within accepted range ... please correct and rerun");
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }
  SPDLOG_DEBUG("`log_period` passed all checks");

  SPDLOG_TRACE("exit Config::validate");
  return {};
}

std::expected<std::filesystem::path, std::string> Config::setup_out(const std::string &id) const noexcept {
  SPDLOG_TRACE("enter Config::setup_dirs");

  std::filesystem::path io_dir;

  try {
    const auto root_dir = out / "out";
    if (!is_directory(root_dir)) {
      SPDLOG_DEBUG("directory `{}` not found ... creating now", root_dir.string());
      create_directory(root_dir);
    } else {
      SPDLOG_DEBUG("directory `{}` already exists", root_dir.string());
    }

    io_dir = root_dir / id;
    if (!is_directory(io_dir)) {
      SPDLOG_DEBUG("directory `{}` not found ... creating now", io_dir.string());
      create_directory(io_dir);
    } else {
      SPDLOG_DEBUG("directory `{}` already exists", io_dir.string());
    }

  } catch (const std::filesystem::filesystem_error &err) {
    const std::string error = fmt::format("unable to create output directory structure: {}", err.what());
    SPDLOG_CRITICAL(error);
    return std::unexpected(error);
  }

  SPDLOG_TRACE("exit Config::setup_dirs with success");
  return io_dir;
}