#include "config.h"

Config::Config(const std::string &input_file_path) {
  SPDLOG_TRACE("enter Config::Config");

  const std::filesystem::path input_file = input_file_path;
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



  parse_time();
  parse_geometry();
  parse_material();
  parse_data();

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

void Config::parse_time() {}

void Config::parse_geometry() {}

void Config::parse_material() {}

void Config::parse_data() {}