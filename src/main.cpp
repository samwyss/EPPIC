#include <filesystem>
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "world.h"

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

  // main io directory setup
  if (!std::filesystem::is_directory("./out/")) {
    console->warn("main io directory `./out/` not found ... creating now");
    try {
      std::filesystem::create_directory("./out/");
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical("unable to create `./out/` directory: {} ",
                           err.what());
      return EXIT_FAILURE;
    }
    console->info("created main io directory `./out/`");
  }

  // run specific directory setup
  const auto io_dir = fmt::format("./out/{}", start_time_str);
  if (!std::filesystem::is_directory(io_dir)) {
    console->warn("run specific io directory `{}` not found ... creating now",
                  io_dir);
    try {
      std::filesystem::create_directory(io_dir);
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical(
          "unable to create run specific io directory `{}`: {} ", io_dir,
          err.what());
      return EXIT_FAILURE;
    }
    console->info("created run specific io directory `{}`", io_dir);
  }

  // logging dir setup
  const auto log_dir = fmt::format("{}/logs", io_dir);
  if (!std::filesystem::is_directory(log_dir)) {
    console->warn("logging directory `{}` not found ... creating now", log_dir);
    try {
      std::filesystem::create_directory(log_dir);
    } catch (const std::filesystem::filesystem_error &err) {
      err_logger->critical("unable to create logging directory `{}`: {} ",
                           log_dir, err.what());
      return EXIT_FAILURE;
    }
    console->info("created logging directory `{}`", log_dir);
  }

  // file based default logger
  const auto logger =
      spdlog::basic_logger_mt("logger", fmt::format("{}/log.log", log_dir));
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::trace);
  spdlog::flush_every(std::chrono::seconds(5));

  console->info("file based logger successfully initialized ... remaining logs "
                "will be written to {}/log.log",
                log_dir);

  // floating point precision
  // todo get this from configuration
  const std::string precision = "single";
  SPDLOG_INFO("floating point precision: {}", precision);

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

  SPDLOG_INFO("EPPIC exiting");
  return EXIT_SUCCESS;
}
