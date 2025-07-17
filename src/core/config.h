#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <simple_xdmf.hpp>
#include <string>
#include <type.h>
#include <type_traits>

#include "hdf5_wrapper.h"

struct Config {
  /// (s) end time of simulation
  fpp end_time;

  /// (m) length of bounding box in the x-direction
  fpp x_len;

  /// (m) length of bounding box in the y-direction
  fpp y_len;

  /// (m) length of bounding box in the z-direction
  fpp z_len;

  /// (Hz) maximum frequency to resolve with FDTD engine
  fpp max_frequency;

  /// number of voxels per minimum wavelength for FDTD engine
  size_t num_vox_min_wavelength;

  /// number of voxels per minimum feature dimension for FDTD engine
  size_t num_vox_min_featur;

  /// diagonally isotropic relative permittivity inside bounding box
  fpp ep_r;

  /// diagonally isotropic relative permeability inside bounding box
  fpp mu_r;

  /// (S / m) diagonally isotropic conductivity of material in bounding box
  fpp sigma;

  /// output directory
  std::filesystem::path out;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio;

  /// output HDF5 file
  HDF5Obj h5;

  /// xdmf writer
  SimpleXdmf xdmf;
};

#endif // CORE_CONFIG_H
