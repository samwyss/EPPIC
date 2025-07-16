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
int main(const int argc, char **argv) {

  const auto start_time = std::chrono::high_resolution_clock::now();
  const auto start_time_str = fmt::format("{:%Y-%m-%d_%H:%M:%S}", start_time);

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
  const auto console = spdlog::stdout_color_mt("stdout");
  const auto err_logger = spdlog::stderr_color_mt("stderr");
  console->info("EPPIC run begin");
#endif

  // io prefix validation
  if (argc < 2) {
#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
    err_logger->critical("io prefix not provided ... please rerun as `EPPIC <io_prefix>`");
#endif
    return EXIT_FAILURE;
  }

  // timestamped io_dir
  std::filesystem::path io_dir;
  // directory and logging setup
  try {
    const auto io_prefix = std::filesystem::canonical(argv[1]);

    const auto root_dir = io_prefix / "out";
    if (!is_directory(root_dir)) {
      create_directory(root_dir);
    }

    io_dir = root_dir / start_time_str;
    if (!is_directory(io_dir)) {
      create_directory(io_dir);
    }

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
    const auto log_dir = io_dir / "logs";
    if (!is_directory(log_dir)) {
      create_directory(log_dir);
    }

    const auto logger = spdlog::basic_logger_mt("logger", (log_dir / "log.log").string());

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace); // needed even for compile time logs
    spdlog::flush_every(std::chrono::seconds(5));

    console->info("file based logger successfully initialized ... remaining logs "
                  "will be written to {}log.log",
                  log_dir.string());
#endif

  } catch (const std::exception &err) {
#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
    err_logger->critical("could not setup directory structure and logger: {}", err.what());
#endif

    return EXIT_FAILURE;
  }

  // todo this will need to be error handled once config is implemented
  auto config = Config();
  config.h5 = HDF5Obj(H5Fcreate((io_dir / "data.h5").c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT),
                      H5Fclose); // todo move to config and create XDMF object elsewhere

  auto world_creation_result = World::create(std::move(config));
  if (!world_creation_result.has_value()) {
    SPDLOG_CRITICAL("failed to configure World object: {}", world_creation_result.error());

    return EXIT_FAILURE;
  }

  auto world = std::move(world_creation_result).value();

  SPDLOG_INFO("configuration successful");

  if (const auto result = world.advance_to(config.end_time); !result.has_value()) {
    SPDLOG_CRITICAL("failed to run EPPIC: {}", result.error());

    return EXIT_FAILURE;
  }

  SPDLOG_INFO("EPPIC exiting");
  return EXIT_SUCCESS;
}
