#include "config.h"

Config::Config(const std::string &input_file_path) {
  // todo input file validation
  parse_time();
  parse_geometry();
  parse_material();
  parse_data();
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