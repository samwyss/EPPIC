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
  /// (Hz) maximum frequency to resolve with FDTD engine
  fpp max_frequency = 15e9;

  /// number of voxels per minimum wavelength for FDTD engine
  size_t num_vox_min_wavelength = 20;

  /// number of voxels per minimum feature dimension for FDTD engine
  size_t num_vox_min_feature = 4;

  /// data output downsampling ratio, number of steps between logged timesteps
  uint64_t ds_ratio = 100000;

  /// (s) end time of simulation
  fpp end_time = 5e-9;

  /// (m) length of bounding box in the x-direction
  fpp x_len = 0.001;

  /// (m) length of bounding box in the y-direction
  fpp y_len = 0.001;

  /// (m) length of bounding box in the z-direction
  fpp z_len = 0.001;

  /// diagonally isotropic relative permittivity inside bounding box
  fpp ep_r = 1.0;

  /// diagonally isotropic relative permeability inside bounding box
  fpp mu_r = 1.0;

  /// (S / m) diagonally isotropic conductivity of material in bounding box
  fpp sigma = 0.0;

  /// output HDF5 file
  HDF5Obj h5;

  /// xdmf writer
  SimpleXdmf xdmf;
};

#endif // CORE_CONFIG_H
