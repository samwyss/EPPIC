#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#include <cstdint>

using namespace std;

struct Config
{
    /// (Hz) maximum frequency to resolve with FDTD engine
    double max_frequency = 15e9;

    /// number of voxels per minimum wavelength for FDTD engine
    size_t num_vox_min_wavelength = 20;

    /// number of voxels per minimum feature dimension for FDTD engine
    size_t num_vox_min_feature = 4;

    /// number of timesteps at which to take a snapshot of selected data streams
    uint64_t num_snapshots = 100;

    /// (s) end time of simulation
    double end_time = 25e-9;

    /// (m) length of bounding box in the x-direction
    double x_len = 0.1;

    /// (m) length of bounding box in the y-direction
    double y_len = 0.1;

    /// (m) length of bounding box in the z-direction
    double z_len = 0.1;

    /// diagonally isotropic relative permittivity inside bounding box
    double ep_r = 1.0;

    /// diagonally isotropic relative permeability inside bounding box
    double mu_r = 1.0;

    /// (S / m) diagonally isotropic conductivity of material in bounding box
    double sigma = 0.0;
};

#endif //CORE_CONFIG_HPP
