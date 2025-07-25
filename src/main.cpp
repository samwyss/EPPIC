#include <chrono>
#include <filesystem>
#include <memory>
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "world.h"

/*!
 * main driver function and build target
 * @param argc argument count
 * @param argv argument vector
 * @return
 */
int main(const int argc, char **argv) {

  const auto start_time = std::chrono::high_resolution_clock::now();
  const auto id = fmt::format("{:%Y-%m-%d_%H:%M:%S}", start_time);

  if (argc < 2) {
    SPDLOG_CRITICAL("io prefix not provided ... please rerun as `EPPIC <cfg_toml_path>`");
    return EXIT_FAILURE;
  }

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
  const auto tmp_log_dir = std::filesystem::current_path() / "logs";

  try {
    if (!is_directory(tmp_log_dir)) {
      create_directory(tmp_log_dir);
    } else {
      SPDLOG_WARN("found previous temporary logging directory at `{}` ... removing now", tmp_log_dir.string());
      std::filesystem::remove_all(tmp_log_dir);
      std::filesystem::create_directory(tmp_log_dir);
    }
  } catch (const std::filesystem::filesystem_error &err) {
    SPDLOG_CRITICAL("unable to create temporary logging directory at `{}`: {}", tmp_log_dir.string(), err.what());
  }
  SPDLOG_DEBUG("created temporary logging directory `{}`", tmp_log_dir.string());

  const auto tmp_logger = spdlog::basic_logger_mt("tmp_logger", (tmp_log_dir / "log.log").string());
  spdlog::set_default_logger(tmp_logger);
  spdlog::set_level(spdlog::level::trace); // needed even for compile time logs
  SPDLOG_DEBUG("created temporary logging directory and logger at `{}`", tmp_log_dir.string());
#endif

  SPDLOG_INFO("EPPIC run begin: {}", start_time);

  std::unique_ptr<World> world;
  try {
    world.reset(new World(argv[1], id));
  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("failed to configure World object: {}", err.what());
    return EXIT_FAILURE;
  }

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
  try {
    const auto log_dir = world->get_output_dir() / "log";

    std::filesystem::rename(tmp_log_dir, log_dir);
    SPDLOG_DEBUG("moved {} to {}", tmp_log_dir.string(), log_dir.string());

    tmp_logger->flush();
    const auto logger = spdlog::basic_logger_mt("logger", (log_dir / "log.log").string(), false);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace); // needed even for compile time logs
    spdlog::flush_every(std::chrono::seconds(5));
    SPDLOG_DEBUG("reset default logger to write to `{}`", (log_dir / "logs.log").string());

  } catch (const std::filesystem::filesystem_error &err) {
    SPDLOG_CRITICAL("unable to move `{}` directory to `{}`: {}", tmp_log_dir.string(),
                    (world->get_output_dir() / "log").string(), err.what());
    return EXIT_FAILURE;
  }

  // NOTE: only used if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto config_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("EPPIC successfully configured: {}", config_time);
  SPDLOG_INFO("elapsed time: {:%H:%M:%S}", config_time - start_time);
  SPDLOG_INFO("begin EPPIC run");
#endif

  if (const auto result = world->run(); !result.has_value()) {
    SPDLOG_CRITICAL("EPPIC run failed: {}", result.error());
    return EXIT_FAILURE;
  }

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
  // NOTE: only used if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
  [[maybe_unused]] const auto run_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("EPPIC run successfully completed: {}", run_time);
  SPDLOG_INFO("elapsed time: {:%H:%M:%S}", run_time - config_time);
  SPDLOG_INFO("total time: {:%H:%M:%S}", run_time - start_time);
#endif

  return EXIT_SUCCESS;
}
