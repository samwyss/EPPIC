/*
 * Copyright (C) 2025 Samuel Wyss
 *
 * This file is part of EPPIC.
 *
 * EPPIC is free software: you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * EPPIC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EPPIC. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef CORE_TYPE_H
#define CORE_TYPE_H

#include <H5Tpublic.h>
#include <type_traits>

/// floating point type (e.g., double or float)
#if EPPIC_USE_FLOAT
using fp_t = float;
#else
using fp_t = double;
#endif

// ensure fp_t is either double or float to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<fp_t, double> || std::is_same_v<fp_t, float>);

/// unsigned integer type (e.g., uint64_t or unit32_t)
#if EPPIC_USE_UINT32_T
using ui_t = uint32_t;
#else
using ui_t = uint64_t;
#endif

/// HDF5 floating point type
template <typename T> hid_t h5_fp_t();

// ensure ui_t is either uint64_t or uint32_t to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<ui_t, uint64_t> || std::is_same_v<ui_t, uint32_t>);

/// HDF5 floating point type template specialization for double
template <> inline hid_t h5_fp_t<double>() { return H5T_NATIVE_DOUBLE; }

/// HDF5 floating point type template specialization for float
template <> inline hid_t h5_fp_t<float>() { return H5T_NATIVE_FLOAT; }

/// HDF5 unsigned integer type
template <typename T> hid_t h5_ui_t();

/// HDF5 unsigned integer type template specialization for uint64_t
template <> inline hid_t h5_ui_t<uint64_t>() { return H5T_NATIVE_UINT64; }

/// HDF5 unsigned integer type template specialization for uint32_t
template <> inline hid_t h5_ui_t<uint32_t>() { return H5T_NATIVE_UINT32; }

#endif // CORE_TYPE_H
