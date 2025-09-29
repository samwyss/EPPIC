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

// ensure ui_t is either uint64_t or uint32_t to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<ui_t, uint64_t> ||
              std::is_same_v<ui_t, uint32_t>); /// floating point type (e.g., double or float)

#endif // CORE_TYPE_H
