#pragma once
#include <numbers>

/**
 * @brief Astron namespace for particle simulation constants \namespace astron::simulation::constants
 */
namespace astron::simulation::constants
{
    // Speed of light in m/s (SI)
    constexpr double C = 299792458.0;
    // Pi constant
    constexpr double PI = std::numbers::pi;
    // Elementary charge in C (SI, exact)
    constexpr double ELEMENTARY_CHARGE = 1.602176634e-19;
    // Particle rest masses in kg (SI)
    constexpr double PROTON_MASS = 1.67262192369e-27;
    constexpr double DEUTERON_MASS = 3.3435837724e-27;
    constexpr double ELECTRON_MASS = 9.1093837139e-31;
    constexpr double ALPHA_PARTICLE_MASS = 6.6446573450e-27;
    // Joules per electron volt
    constexpr double JOULES_PER_EV = ELEMENTARY_CHARGE;
} // namespace astron::simulation::constants