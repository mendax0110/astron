#include <gtest/gtest.h>

#include "simulation/boris_integrator.h"
#include "simulation/field_provider.h"
#include "simulation/particle.h"

#include <cmath>

using namespace astron::simulation;

TEST(ParticleIntegratorTest, GyrationInUniformB)
{
    // Uniform B along z, no E field
    constexpr double Bz = 1.0;
    const UniformField field(Bz);

    Particle p;
    p.m = 1.0;
    p.q = 1.0; // positive charge

    // initial velocity purely in x -> gyration in xy plane
    constexpr double v_perp = 0.5;
    p.v = {v_perp, 0.0, 0.0};
    p.x = {0.0, 0.0, 0.0};

    // cyclotron frequency and period
    const double omega = std::abs(p.q) * Bz / p.m;
    const double period = 2.0 * M_PI / omega;

    const double dt = period / 512.0;
    const int steps = static_cast<int>(std::round(period / dt));

    const double initial_ke = 0.5 * p.m * (p.v[0] * p.v[0] + p.v[1] * p.v[1] + p.v[2] * p.v[2]);

    for (int i = 0; i < steps; ++i)
    {
        BorisIntegrator::step(p, dt, field);
    }

    const double final_ke = 0.5 * p.m * (p.v[0] * p.v[0] + p.v[1] * p.v[1] + p.v[2] * p.v[2]);

    // Energy should be conserved (no E field)
    EXPECT_NEAR(final_ke, initial_ke, 1e-6);

    // After one period, particle should return near starting position (modulo numerical error)
    const double dist = std::sqrt((p.x[0] * p.x[0]) + (p.x[1] * p.x[1]));
    EXPECT_NEAR(dist, 0.0, 5e-3);
}

TEST(ParticleIntegratorTest, RelativisticGyrationEnergyConserved)
{
    constexpr double Bz = 1.0;
    const UniformField field(Bz);

    Particle p;
    p.m = 1.0;
    p.q = 1.0;

    constexpr double c = 299792458.0;
    constexpr double v_perp = 0.5 * c; // rel speed
    p.v = {v_perp, 0.0, 0.0};
    p.x = {0.0, 0.0, 0.0};

    // cyclotron period estimate, non-relativistic would be wrong, but we just step...
    const double omega = std::abs(p.q) * Bz / p.m;
    const double period = 2.0 * M_PI / omega;
    const double dt = period / 512.0;
    const int steps = static_cast<int>(std::round(period / dt));

    auto gamma = [&](const Particle& pr)
    {
        const double v2 = pr.v[0] * pr.v[0] + pr.v[1] * pr.v[1] + pr.v[2] * pr.v[2];
        return 1.0 / std::sqrt(1.0 - v2 / (c * c));
    };

    const double gamma0 = gamma(p);
    for (int i = 0; i < steps; ++i)
        BorisIntegrator::step(p, dt, field, true);
    const double gamma1 = gamma(p);
    EXPECT_NEAR(gamma1, gamma0, 1e-6);
}
