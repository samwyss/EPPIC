#include <filesystem>
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "type.h"
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

  // ensure io prefix is provided
  if (argc < 2) {
    err_logger->critical(
        "io prefix not provided ... please rerun as `EPPIC <io_prefix>`");
    return EXIT_FAILURE;
  }

  // ensure io prefix is a valid path on the filesystem
  const auto io_prefix = std::filesystem::path(argv[1]);
  if (!is_directory(io_prefix)) {
    err_logger->critical("io prefix `{}` is not a valid path on this "
                         "filesystem ... please correct and rerun");
    return EXIT_FAILURE;
  }
  console->info("valid io prefix found `{}`", io_prefix.string());

  // main io dir setup
  const auto out_dir =
      std::filesystem::path(fmt::format("{}/out/", io_prefix.string()));
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
  #if SPDLOG_ACTIVE_LEVEL != SPDLOG_LEVEL_OFF
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
#endif


  // EPPIC configuration
  // todo this will need to be error handled
  auto config = Config();
  config.io_dir = io_dir;

  // EPPIC world
  auto world_creation_result = World::create(config);
  if (!world_creation_result.has_value()) {
    SPDLOG_CRITICAL("failed to configure World object: {}",
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
