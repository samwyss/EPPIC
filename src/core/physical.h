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

#ifndef CORE_PHYSICAL_H
#define CORE_PHYSICAL_H

#include <cmath>
#include <numbers>

#include "type.h"

/// (F/m) vacuum permittivity https://en.wikipedia.org/wiki/Vacuum_permittivity
constexpr fpp VAC_PERMITTIVITY = 8.8541878188e-12;

/// (H/m) vacuum permeability https://en.wikipedia.org/wiki/Vacuum_permeability
constexpr fpp VAC_PERMEABILITY = 4.0 * std::numbers::pi * 1e-7;

/// (m/s) vacuum speed of light
constexpr fpp VAC_SPEED_OF_LIGHT = static_cast<fpp>(1.0 / sqrt(VAC_PERMITTIVITY * VAC_PERMEABILITY));

/// (C) electron charge https://en.wikipedia.org/wiki/Elementary_charge
constexpr fpp ELEC_CHARGE = 1.602176634e-19;

/// (kg) atomic mass unit https://en.wikipedia.org/wiki/Dalton_(unit)
constexpr fpp AMU = 1.66053906892e-27;

/// (kg) electron mass https://en.wikipedia.org/wiki/Electron_mass
constexpr fpp ELEC_MASS = 9.1093837139e-31;

/// (J/K) boltzmann constant https://en.wikipedia.org/wiki/Boltzmann_constant
constexpr fpp BOLTZMANN = 1.380649e-23;

/// (K) electron volt temperature
constexpr fpp EV_TEMP = ELEC_CHARGE / BOLTZMANN;

#endif // CORE_PHYSICAL_H
