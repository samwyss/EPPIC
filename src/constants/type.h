#ifndef CONSTANTS_TYPE_H
#define CONSTANTS_TYPE_H

#include <hdf5/openmpi/hdf5.h>

/// floating point precision (e.g., double or float)
using fpp = float;

// ensure fpp is either double or float to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<fpp, double> || std::is_same_v<fpp, float>);

/// HDF5 floating point precision
using h5fpp =
    std::conditional_t<std::is_same_v<fpp, double>, decltype(H5T_NATIVE_DOUBLE),
                       decltype(H5T_NATIVE_FLOAT)>;

#endif // CONSTANTS_TYPE_H
