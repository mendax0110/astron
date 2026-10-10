#pragma once

#include "field_provider.h"
#include "particle.h"

/**
 * @brief Astron namespace for particle simulation \namespace astron::simulation
 */
namespace astron::simulation
{
    /**
     * @brief Implements the Boris integrator for advancing charged particles in electromagnetic fields. \struct BorisIntegrator
     * @details This struct provides a static method to advance a particle's position and velocity using the Boris algorithm, which is a widely used method for simulating charged
     * particle motion in electromagnetic fields.
     */
    struct BorisIntegrator
    {
        /**
         * @brief Advances a particle's position and velocity using the Boris algorithm.
         * @param p The particle to advance.
         * @param dt The time step for the integration.
         * @param provider The field provider used to obtain electromagnetic fields.
         * @param relativistic Whether to perform the relativistic Boris algorithm.
         */
        static void step(Particle& p, double dt, const FieldProvider& provider, bool relativistic = false);
    };
} // namespace astron::simulation
