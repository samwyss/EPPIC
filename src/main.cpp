#include <mdspan/mdspan.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

#include "world.h"

/*!
 * main driver function and build target
 * @param argc argument count
 * @param argv argument vector
 * @return
 */
int main(int argc, char **argv) {

  // todo
  // finish logging emengine
  // improve logging in main
  // log other constructs
  // fdtd engine
  // hdf5 io
  // particles
  // configuration
  // parallelization

  // spdlog setup
  try {
    // remove old logs
    if (std::filesystem::is_directory("./logs/")) {
      std::filesystem::remove_all("./logs/");
    }

    // create empty logging directory
    std::filesystem::create_directory("./logs/");

    // file based default logger
    const auto logger = spdlog::basic_logger_mt("logger", "./logs/log.log");
    spdlog::set_default_logger(logger);

    // logger options
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(5));

  } catch (const std::exception &err) {
    // temp logger for writing to stderr
    const auto logger = spdlog::stderr_color_mt("stderr");
    logger->critical(err.what());

    return EXIT_FAILURE;
  }

  // initial diagnostics
  SPDLOG_INFO("EPPIC begin");

  // floating point precision
  // todo get this from configuration
  const std::string precision = "double";

  if (precision == "single") {
    // EPPIC configuration
    const auto config = Config<float>();

    // EPPIC world
    auto world_creation_result = World<float>::create(config);
    if (!world_creation_result.has_value()) {
      SPDLOG_CRITICAL("failed to create World object: {}",
                      world_creation_result.error());

      return EXIT_FAILURE;
    }
    auto world = std::move(world_creation_result).value();

    // run EPPIC
    if (auto world_run_result = world.advance_to(config.end_time);
        !world_run_result.has_value()) {
      SPDLOG_CRITICAL("failed to run EPPIC: {}", world_run_result.error());

      return EXIT_FAILURE;
    }

  } else {
    // check for invalid configuration
    if (precision != "double") {
      SPDLOG_WARN("floating point precision `{}` is not supported and will "
                  "default to `double`",
                  precision);
    }

    // EPPIC configuration
    const auto config = Config<double>();

    // EPPIC world
    auto world_creation_result = World<double>::create(config);
    if (!world_creation_result.has_value()) {
      SPDLOG_CRITICAL("failed to create World object: {}",
                      world_creation_result.error());

      return EXIT_FAILURE;
    }
    auto world = std::move(world_creation_result).value();

    // run EPPIC
    if (auto world_run_result = world.advance_to(config.end_time);
        !world_run_result.has_value()) {
      SPDLOG_CRITICAL("failed to run EPPIC: {}", world_run_result.error());

      return EXIT_FAILURE;
    }
  }

  SPDLOG_INFO("EPPIC end");
  return EXIT_SUCCESS;
}
