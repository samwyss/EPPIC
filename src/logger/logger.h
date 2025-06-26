#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <expected>
#include <filesystem>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>

class Logger {
public:
  static std::shared_ptr<spdlog::logger> &get();

  static std::expected<void, std::string> init();

private:
  Logger() = default;

  static std::shared_ptr<spdlog::logger> setup();
};

#endif // LOGGER_LOGGER_H
