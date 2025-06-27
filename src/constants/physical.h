#ifndef CONSTANTS_PHYSICAL_H
#define CONSTANTS_PHYSICAL_H

#include <cmath>
#include <numbers>
#include <type_traits>

/// (F/m) vacuum permittivity https://en.wikipedia.org/wiki/Vacuum_permittivity
template <std::floating_point T>
constexpr T VAC_PERMITTIVITY = static_cast<T>(8.8541878188e-12);

/// (H/m) vacuum permeability https://en.wikipedia.org/wiki/Vacuum_permeability
template <std::floating_point T>
constexpr T VAC_PERMEABILITY = 4.0 * std::numbers::pi * 1e-7;

/// (m/s) vacuum speed of light
template <std::floating_point T>
constexpr T VAC_SPEED_OF_LIGHT =
    static_cast<T>(1.0 / sqrt(VAC_PERMITTIVITY<T> * VAC_PERMEABILITY<T>));

/// (C) electron charge https://en.wikipedia.org/wiki/Elementary_charge
template <std::floating_point T>
constexpr T ELEC_CHARGE = static_cast<T>(1.602176634e-19);

/// (kg) atomic mass unit https://en.wikipedia.org/wiki/Dalton_(unit)
template <std::floating_point T>
constexpr T AMU = static_cast<T>(1.66053906892e-27);

/// (kg) electron mass https://en.wikipedia.org/wiki/Electron_mass
template <std::floating_point T>
constexpr T ELEC_MASS = static_cast<T>(9.1093837139e-31);

/// (J/K) boltzmann constant https://en.wikipedia.org/wiki/Boltzmann_constant
template <std::floating_point T>
constexpr T BOLTZMANN = static_cast<T>(1.380649e-23);

/// (K) electron volt temperature
template <std::floating_point T>
constexpr T EV_TEMP = static_cast<T>(ELEC_CHARGE<T> / BOLTZMANN<T>);

#endif // CONSTANTS_PHYSICAL_HP
