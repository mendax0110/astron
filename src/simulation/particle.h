#pragma once

#include <array>

/**
 * @brief Astron namespace for particle simulation \namespace astron::simulation
 */
namespace astron::simulation
{
    /**
     * @brief Represents a charged particle with position, velocity, mass, and charge. \struct Particle
     */
    struct Particle
    {
        std::array<double, 3> x{{0.0, 0.0, 0.0}};
        std::array<double, 3> v{{0.0, 0.0, 0.0}};
        double m = 1.0;
        double q = 1.0;
    };
}
