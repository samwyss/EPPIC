#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <type_traits>

#include <type.h>

struct Config {
  /// (Hz) maximum frequency to resolve with FDTD engine
  fpp max_frequency = 15e9;

  /// number of voxels per minimum wavelength for FDTD engine
  size_t num_vox_min_wavelength = 20;

  /// number of voxels per minimum feature dimension for FDTD engine
  size_t num_vox_min_feature = 4;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio = 10;

  /// (s) end time of simulation
  /// todo revert back to 25e-9
  fpp end_time = 1e-9;

  /// (m) length of bounding box in the x-direction
  /// todo revert back to 0.1
  fpp x_len = 0.001;

  /// (m) length of bounding box in the y-direction
  /// todo revert back to 0.1
  fpp y_len = 0.001;

  /// (m) length of bounding box in the z-direction
  /// todo revert back to 0.1
  fpp z_len = 0.001;

  /// diagonally isotropic relative permittivity inside bounding box
  fpp ep_r = 1.0;

  /// diagonally isotropic relative permeability inside bounding box
  fpp mu_r = 1.0;

  /// (S / m) diagonally isotropic conductivity of material in bounding box
  fpp sigma = 0.0;

  /// io directory
  std::filesystem::path io_dir;
};

#endif // CORE_CONFIG_H
