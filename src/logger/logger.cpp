#include "logger.h"

std::shared_ptr<spdlog::logger> &Logger::get() {
  static std::shared_ptr<spdlog::logger> logger = setup();
  return logger;
}

std::expected<void, std::string> Logger::init() {
  // todo improve me later
  if (std::filesystem::is_directory("./logs/")) {
    std::filesystem::remove_all("./logs/");
  }
  std::filesystem::create_directory("./logs/");

  return {};
}

std::shared_ptr<spdlog::logger> Logger::setup() {
  std::string name = "logger";
  std::string file = "./logs/log.log";

  auto logger = spdlog::basic_logger_mt(name, file);

  // todo add a way to set this at run or compile time
  logger->set_level(spdlog::level::info);

  // todo add formatting for log message

  return logger;
}
