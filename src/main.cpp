#include <filesystem>
#include <fmt/chrono.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "type.h"
#include "world.h"

#include <sys/stat.h>

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
    SPDLOG_CRITICAL("io prefix not provided ... please rerun as `EPPIC <io_prefix>`");
    return EXIT_FAILURE;
  }

#if SPDLOG_ACTIVE_LEVEL < SPDLOG_LEVEL_OFF
  const auto tmp_log_dir = std::filesystem::canonical("./log");

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

  const auto tmp_logger = spdlog::basic_logger_mt("logger", (tmp_log_dir / "log.log").string());
  spdlog::set_default_logger(tmp_logger);
  spdlog::set_level(spdlog::level::trace); // needed even for compile time logs
#endif

  SPDLOG_INFO("EPPIC run begin: {}", start_time);

  auto config_result = Config::create(argv[1], id);
  if (!config_result.has_value()) {
    SPDLOG_CRITICAL("failed to configure EPPIC: {}", config_result.error());
    return EXIT_FAILURE;
  }
  auto config = std::move(config_result.value());

  try {
    std::filesystem::rename(tmp_log_dir, config.out / "log");
    SPDLOG_DEBUG("moved {} to {}", tmp_log_dir.string(), (config.out / "log").string());

    const auto logger = spdlog::basic_logger_mt("logger", (config.out / "logs/log.log").string(), true);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace); // needed even for compile time logs
    spdlog::flush_every(std::chrono::seconds(5));
    SPDLOG_DEBUG("reset default logger to write to `{}`", (config.out / "logs/log.log").string());
  } catch (const std::filesystem::filesystem_error &err) {
    SPDLOG_CRITICAL("unable to move `{}` directory to `{}`: {}", tmp_log_dir.string(), (config.out / "log").string(),
                    err.what());
    return EXIT_FAILURE;
  }

  // todo need to rewrite world to be composed of moved config
  auto world_creation_result = World::create(std::move(config));
  if (!world_creation_result.has_value()) {
    SPDLOG_CRITICAL("failed to configure World object: {}", world_creation_result.error());
    return EXIT_FAILURE;
  }
  auto world = std::move(world_creation_result).value();

  const auto config_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("EPPIC successfully configured: {}", config_time);
  SPDLOG_INFO("elapsed time (s): {}", config_time - start_time);

  if (const auto result = world.advance_to(config.end_time); !result.has_value()) {
    SPDLOG_CRITICAL("EPPIC run failed: {}", result.error());
    return EXIT_FAILURE;
  }

  const auto run_time = std::chrono::high_resolution_clock::now();
  SPDLOG_INFO("EPPIC run successfully completed: {}", run_time);
  SPDLOG_INFO("elapsed time (s): {}", run_time - config_time);
  SPDLOG_INFO("total time (s): {}", run_time - start_time);

  return EXIT_SUCCESS;
}
