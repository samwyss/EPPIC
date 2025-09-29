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

/// floating point precision (e.g., double or float)
using fpp = double;

// ensure fpp is either double or float to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<fpp, double> || std::is_same_v<fpp, float>);

#endif // CORE_TYPE_H
