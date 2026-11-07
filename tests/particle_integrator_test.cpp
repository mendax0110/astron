#include <gtest/gtest.h>

#include "simulation/boris_integrator.h"
#include "simulation/field_provider.h"
#include "simulation/particle.h"

#include <cmath>
#include <memory>
#include <numbers>

using namespace astron::simulation;

namespace
{
    double vectorMagnitude(const std::array<double, 3>& value)
    {
        return std::sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2]);
    }

    double uniformOrbitReturnError(const int steps)
    {
        const UniformField field(1.0);
        Particle particle;
        particle.m = 1.0;
        particle.q = 1.0;
        particle.v = {0.5, 0.0, 0.0};

        const double period = 2.0 * std::numbers::pi;
        const double dt = period / steps;
        for (int step = 0; step < steps; ++step)
        {
            BorisIntegrator::step(particle, dt, field);
        }

        return std::sqrt(vectorMagnitude(particle.x) * vectorMagnitude(particle.x) + (particle.v[0] - 0.5) * (particle.v[0] - 0.5) + particle.v[1] * particle.v[1] +
                         particle.v[2] * particle.v[2]);
    }
} // namespace

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
    constexpr double v_perp = 0.6 * c;
    p.v = {v_perp, 0.0, 0.0};
    p.x = {0.0, 0.0, 0.0};

    auto gamma = [&](const Particle& pr)
    {
        const double v2 = pr.v[0] * pr.v[0] + pr.v[1] * pr.v[1] + pr.v[2] * pr.v[2];
        return 1.0 / std::sqrt(1.0 - v2 / (c * c));
    };

    const double gamma0 = gamma(p);
    const double relativisticPeriod = 2.0 * std::numbers::pi * gamma0 * p.m / (std::abs(p.q) * Bz);
    constexpr int steps = 2048;
    const double dt = relativisticPeriod / steps;
    for (int i = 0; i < steps; ++i)
    {
        BorisIntegrator::step(p, dt, field, true);
    }
    const double gamma1 = gamma(p);
    EXPECT_NEAR(gamma1, gamma0, 1e-6);
    EXPECT_NEAR(p.v[0] / v_perp, 1.0, 1e-5);
    EXPECT_NEAR(p.v[1] / v_perp, 0.0, 1e-5);
}

TEST(ParticleIntegratorTest, UniformMagneticOrbitConvergesWithSmallerTimestep)
{
    const double coarseError = uniformOrbitReturnError(32);
    const double mediumError = uniformOrbitReturnError(64);
    const double fineError = uniformOrbitReturnError(128);

    EXPECT_LT(mediumError, coarseError);
    EXPECT_LT(fineError, mediumError);
    EXPECT_GT(coarseError / fineError, 3.0);
}

TEST(ParticleIntegratorTest, UniformElectricAndMagneticFieldsProduceExBDrift)
{
    constexpr double magneticStrength = 1.0;
    constexpr double electricStrength = 0.2;
    auto magnetic = std::make_shared<const UniformField>(magneticStrength);
    const UniformElectricFieldProvider field(magnetic, {electricStrength, 0.0, 0.0});

    Particle particle;
    particle.m = 1.0;
    particle.q = 1.0;
    particle.v = {0.0, -electricStrength / magneticStrength, 0.0};

    const double period = 2.0 * std::numbers::pi;
    const double dt = period / 128.0;
    constexpr int periods = 100;
    constexpr int stepsPerPeriod = 128;
    for (int step = 0; step < periods * stepsPerPeriod; ++step)
    {
        BorisIntegrator::step(particle, dt, field);
    }

    const double elapsed = periods * period;
    const double driftX = particle.x[0] / elapsed;
    const double driftY = particle.x[1] / elapsed;
    EXPECT_NEAR(driftX, 0.0, 1e-5);
    EXPECT_NEAR(driftY, -electricStrength / magneticStrength, 2e-4);
}
