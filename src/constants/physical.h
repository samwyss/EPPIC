#ifndef CONSTANTS_PHYSICAL_HPP
#define CONSTANTS_PHYSICAL_HPP

# include <numbers>

using namespace std::numbers;

/// (F / m) vacuum permittivity https://en.wikipedia.org/wiki/Vacuum_permittivity
constexpr double VAC_PERMITTIVITY = 8.8541878188e-12;

/// (H / m) vacuum permeability https://en.wikipedia.org/wiki/Vacuum_permeability
constexpr double VAC_PERMEABILITY = 4.0 * std::numbers::pi * 1e-7;

/// (C) electron charge https://en.wikipedia.org/wiki/Elementary_charge
constexpr double ELEC_CHARGE = 1.602176634e-19;

/// (kg) atomic mass unit https://en.wikipedia.org/wiki/Dalton_(unit)
constexpr double AMU = 1.66053906892e-27;

/// (kg) electron mass https://en.wikipedia.org/wiki/Electron_mass
constexpr double ELEC_MASS = 9.1093837139e-31;

/// (J / K) boltzmann constant https://en.wikipedia.org/wiki/Boltzmann_constant
constexpr double BOLTZMANN = 1.380649e-23;

/// (K) electron volt temperature
constexpr double EV_TEMP = ELEC_CHARGE / BOLTZMANN;

#endif //CONSTANTS_PHYSICAL_HPP
