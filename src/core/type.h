#ifndef CORE_TYPE_H
#define CORE_TYPE_H

/// floating point precision (e.g., double or float)
using fpp = double;

// ensure fpp is either double or float to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<fpp, double> || std::is_same_v<fpp, float>);

#endif // CORE_TYPE_H
