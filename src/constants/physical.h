#ifndef CONSTANTS_PHYSICAL_H
#define CONSTANTS_PHYSICAL_H

#include <cmath>
#include <numbers>

#include "type.h"

/// (F/m) vacuum permittivity https://en.wikipedia.org/wiki/Vacuum_permittivity
constexpr fpp VAC_PERMITTIVITY = 8.8541878188e-12;

/// (H/m) vacuum permeability https://en.wikipedia.org/wiki/Vacuum_permeability
constexpr fpp VAC_PERMEABILITY = 4.0 * std::numbers::pi * 1e-7;

/// (m/s) vacuum speed of light
constexpr fpp VAC_SPEED_OF_LIGHT =
    static_cast<fpp>(1.0 / sqrt(VAC_PERMITTIVITY * VAC_PERMEABILITY));

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

#endif // CONSTANTS_PHYSICAL_HP
