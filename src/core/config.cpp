#include "config.h"

Config::Config(const std::string &input_file_path) {
  SPDLOG_TRACE("enter Config::Config");

  const std::filesystem::path input_file = std::filesystem::canonical(input_file_path);
  if (!std::filesystem::exists(input_file)) {
    SPDLOG_CRITICAL("input file path `{}` not found", input_file_path);
    throw std::runtime_error(
        fmt::format("input file path `{}` not found ... please correct and rerun", input_file.string()));
  }
  if (!std::filesystem::is_regular_file(input_file)) {
    SPDLOG_CRITICAL("input file path `{}` is not a regular file", input_file_path);
    if (!std::filesystem::is_directory(input_file)) {
      SPDLOG_CRITICAL("input file path `{}` is a directory not a file", input_file_path);
      throw std::runtime_error(fmt::format(
          "input file path `{}` is a directory not a file ... please correct and rerun", input_file.string()));
    }
    throw std::runtime_error(
        fmt::format("input file path `{}` is not regular file ... please correct and rerun", input_file.string()));
  }
  SPDLOG_DEBUG("input file path `{}` successfully verified", input_file.string());

  if (const auto parse_result = toml::try_parse(input_file); parse_result.is_ok()) {
    SPDLOG_INFO("input file `{}` is valid toml");

    const auto config = parse_result.unwrap();

    if (const auto result = parse_time(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [time] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [time] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [time] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_geometry(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [geometry] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [geometry] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [geometry] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_material(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [material] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [material] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [material] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_time(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [data] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [data] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [data] section in `{}`: {}", input_file.string(), result.error()));
    }

  } else {
    SPDLOG_CRITICAL("failed to parse config file `{}`: {}", input_file.string(), parse_result.unwrap_err());
    throw std::runtime_error(
        fmt::format("failed to parse config file `{}`: {}", input_file.string(), parse_result.unwrap_err()));
  }

  SPDLOG_TRACE("exit Config::Config");
}

std::expected<Config, std::string> Config::create(const std::string &input_file_path) {
  SPDLOG_TRACE("enter Config::create");
  try {
    Config config(input_file_path);
    SPDLOG_TRACE("exit Config::create with success");
    return config;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit Config::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> Config::parse_time(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_time");
  try {
    end_time = toml::find<fpp>(config, "time", "end_time");
    if (end_time <= 0.0) {
      throw std::invalid_argument(
          fmt::format("`end_time` must be non-zero and non-negative ... value was `{}`", end_time));
    }
  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("parsing [time] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_time with success");
  return {};
}

std::expected<void, std::string> Config::parse_geometry(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_geometry");

  try {
    x_len = toml::find<fpp>(config, "geometry", "x_len");
    if (x_len <= 0.0) {
      throw std::invalid_argument(fmt::format("`x_len` must be non-zero and non-negative ... value was `{}`", x_len));
    }

    y_len = toml::find<fpp>(config, "geometry", "y_len");
    if (y_len <= 0.0) {
      throw std::invalid_argument(fmt::format("`y_len` must be non-zero and non-negative ... value was `{}`", y_len));
    }

    z_len = toml::find<fpp>(config, "geometry", "z_len");
    if (z_len <= 0.0) {
      throw std::invalid_argument(fmt::format("`z_len` must be non-zero and non-negative ... value was `{}`", z_len));
    }

    // todo you are here, need to finish parsing and throw in debug statements into range checking and success
  } catch (const toml::type_error &err) {
    SPDLOG_CRITICAL("parsing [geometry] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_geometry with success");
  return {};
}

std::expected<void, std::string> Config::parse_material(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_material");

  try {

  } catch (const toml::type_error &err) {
    SPDLOG_CRITICAL("parsing [material] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_material with success");
  return {};
}

std::expected<void, std::string> Config::parse_data(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_data");

  try {

  } catch (const toml::type_error &err) {
    SPDLOG_CRITICAL("parsing [data] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_data with success");
  return {};
}