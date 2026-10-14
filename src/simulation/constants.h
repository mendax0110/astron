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
} // namespace astron::simulation::constants