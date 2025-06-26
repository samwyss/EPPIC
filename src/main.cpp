#include <mdspan/mdspan.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>

#include "scalar.h"
#include "vector.h"
#include "world.h"

/*!
 * main driver function and build target
 * @param argc argument count
 * @param argv argument vector
 * @return
 */
int main(int argc, char **argv) {

  // todo
  // logging / diagnostics
  // improved error handling
  // fdtd engine
  // hdf5 io
  // particles
  // configuration
  // parallelization

  // todo get this from configuration
  const std::string fp_precision = "double";

  // auto _ = Logger::init();
  // auto logger = Logger::get();

  // initial diagnostics
  // logger->info("EPPIC run starting");

  // setup spdlog
  if (std::filesystem::is_directory("./logs/")) {
    std::filesystem::remove_all("./logs/");
  }
  std::filesystem::create_directory("./logs/");
  const auto logger = spdlog::basic_logger_mt("logger", "./logs/log.log");
  logger->set_level(spdlog::level::trace);
  spdlog::set_default_logger(logger);

  if (fp_precision == "double") {
    const auto config = Config<double>();
    auto world = World<double>::create(config).value();
    world.advance_to(config.end_time).value();
    SPDLOG_CRITICAL("critical");
    SPDLOG_ERROR("error");
    SPDLOG_WARN("warn");
    SPDLOG_INFO("info");
    SPDLOG_DEBUG("debug");
    SPDLOG_TRACE("trace");
    SPDLOG_CRITICAL("critical");
    SPDLOG_ERROR("error");
    SPDLOG_WARN("warn");
    SPDLOG_INFO("info");
    SPDLOG_DEBUG("debug");
    SPDLOG_TRACE("trace");
  } else if (fp_precision == "single") {
    const auto config = Config<float>();
    auto world = World<float>::create(config).value();
    world.advance_to(config.end_time).value();
  } else {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
