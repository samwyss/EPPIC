#include <filesystem>
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "world.h"

// floating point precision (double, float)
using precision = float;

/*!
 * main driver function and build target
 * @param argc argument count
 * @param argv argument vector
 * @return
 */
int main(int argc, char **argv) {

  // (s) EPPIC start time
  const auto start_time = std::chrono::high_resolution_clock::now();

  // start time as std::string
  const auto start_time_str = fmt::format("{:%Y-%m-%d_%H:%M:%S}", start_time);

  // temporary logger to stdio
  const auto console = spdlog::stdout_color_mt("console");

  // temporary logger to stderr
  const auto err_logger = spdlog::stderr_color_mt("stderr");

  console->info("EPPIC run begin");

  // main io dir setup
  const auto out_dir = std::filesystem::path("./out/");
  if (!is_directory(out_dir)) {
    console->warn("directory `{}` not found ... creating now",
                  out_dir.string());
    try {
      create_directory(out_dir);
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical("unable to create `{}` directory: {} ",
                           out_dir.string(), err.what());
      return EXIT_FAILURE;
    }
    console->info("created directory `{}`", out_dir.string());
  }

  // timestamped io dir setup
  const auto io_dir = std::filesystem::path(
      fmt::format("{}{}/", out_dir.string(), start_time_str));
  if (!is_directory(io_dir)) {
    console->warn("directory `{}` not found ... creating now", io_dir.string());
    try {
      create_directory(io_dir);
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical("unable to create directory `{}`: {} ",
                           io_dir.string(), err.what());
      return EXIT_FAILURE;
    }
    console->info("created directory `{}`", io_dir.string());
  }

  // logging dir setup
  const auto log_dir =
      std::filesystem::path(fmt::format("{}logs/", io_dir.string()));
  if (!is_directory(log_dir)) {
    console->warn("directory `{}` not found ... creating now",
                  log_dir.string());
    try {
      create_directory(log_dir);
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical("unable to create directory `{}`: {} ",
                           log_dir.string(), err.what());
      return EXIT_FAILURE;
    }
    console->info("created directory `{}`", log_dir.string());
  }

  // file based default logger
  const auto logger = spdlog::basic_logger_mt(
      "logger", fmt::format("{}log.log", log_dir.string()));
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::trace);
  spdlog::flush_every(std::chrono::seconds(5));

  console->info("file based logger successfully initialized ... remaining logs "
                "will be written to {}log.log",
                log_dir.string());

  // EPPIC configuration
  auto config = Config<precision>();
  config.io_dir = io_dir;

  // EPPIC world
  auto world_creation_result = World<precision>::create(config);
  if (!world_creation_result.has_value()) {
    SPDLOG_CRITICAL("failed to create World object: {}",
                    world_creation_result.error());

    return EXIT_FAILURE;
  }
  auto world = std::move(world_creation_result).value();

  SPDLOG_INFO("configuration successful");

  // run EPPIC
  if (auto result = world.advance_to(config.end_time); !result.has_value()) {
    SPDLOG_CRITICAL("failed to run EPPIC: {}", result.error());

    return EXIT_FAILURE;
  }

  SPDLOG_INFO("EPPIC exiting");
  return EXIT_SUCCESS;
}
