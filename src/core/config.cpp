#include "config.h"

Config::Config(const std::string &input_file_path, const std::string &id) {
  SPDLOG_TRACE("enter Config::Config");

  const std::filesystem::path input_file = std::filesystem::canonical(input_file_path);
  if (!std::filesystem::exists(input_file)) {
    SPDLOG_CRITICAL("input file path `{}` not found", input_file_path);
    throw std::runtime_error(
        fmt::format("input file path `{}` not found ... please correct and rerun", input_file.string()));
  }
  if (!std::filesystem::is_regular_file(input_file)) {
    SPDLOG_CRITICAL("input file path `{}` is not a regular file", input_file_path);
    if (!std::filesystem::is_directory(input_file)) {
      SPDLOG_CRITICAL("input file path `{}` is a directory not a file", input_file_path);
      throw std::runtime_error(fmt::format(
          "input file path `{}` is a directory not a file ... please correct and rerun", input_file.string()));
    }
    throw std::runtime_error(
        fmt::format("input file path `{}` is not regular file ... please correct and rerun", input_file.string()));
  }
  SPDLOG_DEBUG("input file path `{}` successfully verified", input_file.string());

  if (const auto parse_result = toml::try_parse(input_file); parse_result.is_ok()) {
    SPDLOG_INFO("input file `{}` is valid toml");

    const auto &config = parse_result.unwrap();

    if (const auto result = parse_time(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [time] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [time] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [time] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_geometry(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [geometry] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [geometry] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [geometry] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_material(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [material] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [material] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [material] section in `{}`: {}", input_file.string(), result.error()));
    }

    if (const auto result = parse_time(config); result.has_value()) {
      SPDLOG_DEBUG("successfully parsed [data] section in `{}`", input_file.string());
    } else {
      SPDLOG_CRITICAL("cannot parse [data] section in `{}`: {}", input_file.string(), result.error());
      throw std::runtime_error(
          fmt::format("cannot parse [data] section in `{}`: {}", input_file.string(), result.error()));
    }

  } else {
    SPDLOG_CRITICAL("failed to parse config file `{}`: {}", input_file.string(), parse_result.unwrap_err());
    throw std::runtime_error(
        fmt::format("failed to parse config file `{}`: {}", input_file.string(), parse_result.unwrap_err()));
  }

  SPDLOG_TRACE("exit Config::Config");
}

std::expected<Config, std::string> Config::create(const std::string &input_file_path, const std::string &id) {
  SPDLOG_TRACE("enter Config::create");
  try {
    Config config(input_file_path, id);
    SPDLOG_TRACE("exit Config::create with success");
    return config;
  } catch (const std::runtime_error &err) {
    SPDLOG_CRITICAL("exit Config::create with error: {}", err.what());
    return std::unexpected(err.what());
  }
}

std::expected<void, std::string> Config::parse_time(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_time");
  try {
    end_time = toml::find<fpp>(config, "time", "end_time");
    SPDLOG_DEBUG("`end_time` successfully parsed as fpp with value `{}`", end_time);
    if (end_time <= 0.0) {
      SPDLOG_CRITICAL("`end_time` must be non-zero and non-negative ... value was `{}`", end_time);
      throw std::invalid_argument(
          fmt::format("`end_time` must be non-zero and non-negative ... value was `{}`", end_time));
    }
    SPDLOG_DEBUG("`end_time` passed all checks");

  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("parsing [time] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_time with success");
  return {};
}

std::expected<void, std::string> Config::parse_geometry(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_geometry");

  try {
    x_len = toml::find<fpp>(config, "geometry", "x_len");
    SPDLOG_DEBUG("`x_len` successfully parsed as fpp with value `{}`", x_len);
    if (x_len <= 0.0) {
      SPDLOG_CRITICAL("`x_len` must be non-zero and non-negative ... value was `{}`", x_len);
      throw std::invalid_argument(fmt::format("`x_len` must be non-zero and non-negative ... value was `{}`", x_len));
    }
    SPDLOG_DEBUG("`x_len` passed all checks");

    y_len = toml::find<fpp>(config, "geometry", "y_len");
    SPDLOG_DEBUG("`y_len` successfully parsed as fpp with value `{}`", y_len);
    if (y_len <= 0.0) {
      SPDLOG_CRITICAL("`y_len` must be non-zero and non-negative ... value was `{}`", y_len);
      throw std::invalid_argument(fmt::format("`y_len` must be non-zero and non-negative ... value was `{}`", y_len));
    }
    SPDLOG_DEBUG("`y_len` passed all checks");

    z_len = toml::find<fpp>(config, "geometry", "z_len");
    SPDLOG_DEBUG("`z_len` successfully parsed as fpp with value `{}`", z_len);
    if (z_len <= 0.0) {
      SPDLOG_CRITICAL("`z_len` must be non-zero and non-negative ... value was `{}`", z_len);
      throw std::invalid_argument(fmt::format("`z_len` must be non-zero and non-negative ... value was `{}`", z_len));
    }
    SPDLOG_DEBUG("`z_len` passed all checks");

    max_frequency = toml::find<fpp>(config, "geometry", "max_frequency");
    SPDLOG_DEBUG("`max_frequency` successfully parsed as fpp with value `{}`", max_frequency);
    if (max_frequency <= 0.0) {
      SPDLOG_CRITICAL("`max_frequency` must be non-zero and non-negative ... value was `{}`", max_frequency);
      throw std::invalid_argument(
          fmt::format("`max_frequency` must be non-zero and non-negative ... value was `{}`", max_frequency));
    }
    SPDLOG_DEBUG("`max_frequency` passed all checks");

    num_vox_min_wavelength = toml::find<size_t>(config, "geometry", "num_vox_min_wavelength");
    SPDLOG_DEBUG("`num_vox_min_wavelength` successfully parsed as size_t with value `{}`", num_vox_min_wavelength);
    if (num_vox_min_wavelength <= 0) {
      SPDLOG_CRITICAL("`num_vox_min_wavelength` must be non-zero and non-negative ... value was `{}`",
                      num_vox_min_wavelength);
      throw std::invalid_argument(fmt::format(
          "`num_vox_min_wavelength` must be non-zero and non-negative ... value was `{}`", num_vox_min_wavelength));
    }
    SPDLOG_DEBUG("`num_vox_min_wavelength` passed all checks");

    num_vox_min_feature = toml::find<size_t>(config, "geometry", "num_vox_min_feature");
    SPDLOG_DEBUG("`num_vox_min_feature` successfully parsed as size_t with value `{}`", num_vox_min_feature);
    if (num_vox_min_feature <= 0) {
      SPDLOG_CRITICAL("`num_vox_min_feature` must be non-zero and non-negative ... value was `{}`",
                      num_vox_min_feature);
      throw std::invalid_argument(fmt::format(
          "`num_vox_min_feature` must be non-zero and non-negative ... value was `{}`", num_vox_min_feature));
    }
    SPDLOG_DEBUG("`num_vox_min_feature` passed all checks");

  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("parsing [geometry] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_geometry with success");
  return {};
}

std::expected<void, std::string> Config::parse_material(const toml::basic_value<toml::type_config> &config) {
  SPDLOG_TRACE("enter Config::parse_material");

  try {
    ep_r = toml::find<fpp>(config, "material", "ep_r");
    SPDLOG_DEBUG("`ep_r` successfully parsed as fpp with value `{}`", ep_r);
    if (ep_r <= 0.0) {
      SPDLOG_CRITICAL("`ep_r` must be non-zero and non-negative ... value was `{}`", ep_r);
      throw std::invalid_argument(fmt::format("`ep_r` must be non-zero and non-negative ... value was `{}`", ep_r));
    }
    SPDLOG_DEBUG("`ep_r` passed all checks");

    mu_r = toml::find<fpp>(config, "material", "mu_r");
    SPDLOG_DEBUG("`mu_r` successfully parsed as fpp with value `{}`", mu_r);
    if (mu_r <= 0.0) {
      SPDLOG_CRITICAL("`mu_r` must be non-zero and non-negative ... value was `{}`", mu_r);
      throw std::invalid_argument(fmt::format("`mu_r` must be non-zero and non-negative ... value was `{}`", mu_r));
    }
    SPDLOG_DEBUG("`mu_r` passed all checks");

    sigma = toml::find<fpp>(config, "material", "sigma");
    SPDLOG_DEBUG("`sigma successfully parsed as fpp with value `{}`", sigma);
    if (sigma <= 0.0) {
      SPDLOG_CRITICAL("`sigma` must be non-zero and non-negative ... value was `{}`", sigma);
      throw std::invalid_argument(fmt::format("`sigma` must be non-zero and non-negative ... value was `{}`", sigma));
    }
    SPDLOG_DEBUG("`sigma` passed all checks");

  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("parsing [material] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_material with success");
  return {};
}

std::expected<void, std::string> Config::parse_data(const toml::basic_value<toml::type_config> &config,
                                                    const std::string &id) {
  SPDLOG_TRACE("enter Config::parse_data");

  try {
    const auto out_dir_str = toml::find<std::string>(config, "data", "out_dir");
    SPDLOG_DEBUG("`out_dir` successfully parsed as std::string with value `{}`", out_dir_str);
    const auto out_dir = std::filesystem::canonical(out_dir_str);
    SPDLOG_DEBUG("`out_dir` successfully canonicalized with value`{}`", out_dir.string());
    if (const auto result = setup_dirs(out_dir, id); result.has_value()) {
      out = result.value();
      SPDLOG_DEBUG("`out_dir` passed all checks");
      SPDLOG_INFO("created output directory structure at `{}`", out.string());
    } else {
      SPDLOG_CRITICAL("unable to create output directory structure: {}", result.error());
      return std::unexpected(result.error());
    }

    ds_ratio = toml::find<uint64_t>(config, "data", "ds_ratio");
    SPDLOG_DEBUG("`ds_ratio` successfully parsed as uint64_t with value `{}`", ds_ratio);
    if (ds_ratio <= 0) {
      SPDLOG_CRITICAL("`ds_ratio` must be non-zero and non-negative ... value was `{}`", ds_ratio);
      throw std::invalid_argument(fmt::format("`ds_ratio` must be non-zero and non-negative ... value was `{}`", ds_ratio));
    }
    SPDLOG_DEBUG("`ds_ratio` passed all checks");

  } catch (const std::exception &err) {
    SPDLOG_CRITICAL("parsing [data] section failed: {}", err.what());
    return std::unexpected(err.what());
  }

  SPDLOG_TRACE("exit Config::parse_data with success");
  return {};
}

std::expected<std::filesystem::path, std::string> Config::setup_dirs(const std::filesystem::path &out_dir,
                                                                     const std::string &id) {
  SPDLOG_TRACE("enter Config::setup_dirs");

  std::filesystem::path io_dir;

  try {
    const auto root_dir = out_dir / "out";
    if (!is_directory(root_dir)) {
      SPDLOG_DEBUG("directory `{}` not found ... creating now", root_dir);
      create_directory(root_dir);
    } else {
      SPDLOG_DEBUG("directory `{}` already exists", root_dir);
    }

    io_dir = root_dir / id;
    if (!is_directory(io_dir)) {
      SPDLOG_DEBUG("directory `{}` not found ... creating now", io_dir);
      create_directory(io_dir);
    } else {
      SPDLOG_DEBUG("directory `{}` already exists", io_dir);
    }

  } catch (const std::filesystem::filesystem_error &err) {
    SPDLOG_CRITICAL("exit Config::create_dirs with failure: {}", err.what());
    return std::unexpected<std::string>(err.what());
  }

  SPDLOG_TRACE("exit Config::setup_dirs with success");
  return io_dir;
}