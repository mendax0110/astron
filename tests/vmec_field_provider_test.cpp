#include "simulation/vmec_field_provider.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

using namespace astron;

namespace
{
    core::Equilibrium circularEquilibrium()
    {
        core::Equilibrium equilibrium;
        equilibrium.fieldPeriods = 1;
        equilibrium.radialSurfaces = 5;
        equilibrium.modes = {{.m = 0.0, .n = 0.0}, {.m = 1.0, .n = 0.0}};
        equilibrium.fieldModes = {{.m = 0.0, .n = 0.0}};
        equilibrium.radiusCos.resize(10);
        equilibrium.heightSin.resize(10);
        equilibrium.bThetaCos.assign(5, 0.0);
        equilibrium.bPhiCos.assign(5, 1.0);
        equilibrium.bStrengthCos.assign(5, 1.0);

        for (int surface = 0; surface < equilibrium.radialSurfaces; ++surface)
        {
            const double rho = std::sqrt(static_cast<double>(surface) / (equilibrium.radialSurfaces - 1));
            const size_t offset = static_cast<size_t>(surface) * equilibrium.modes.size();
            equilibrium.radiusCos[offset] = 5.0;
            equilibrium.radiusCos[offset + 1] = rho;
            equilibrium.heightSin[offset + 1] = rho;
        }

        return equilibrium;
    }

    core::Equilibrium varyingEquilibrium()
    {
        core::Equilibrium equilibrium;
        equilibrium.fieldPeriods = 1;
        equilibrium.radialSurfaces = 9;
        equilibrium.modes = {{.m = 0.0, .n = 0.0}, {.m = 1.0, .n = 0.0}, {.m = 2.0, .n = 1.0}};
        equilibrium.fieldModes = equilibrium.modes;
        const size_t count = static_cast<size_t>(equilibrium.radialSurfaces) * equilibrium.modes.size();
        equilibrium.radiusCos.assign(count, 0.0);
        equilibrium.heightSin.assign(count, 0.0);
        equilibrium.bThetaCos.assign(count, 0.0);
        equilibrium.bPhiCos.assign(count, 0.0);
        equilibrium.bStrengthCos.assign(count, 0.0);

        for (int surface = 0; surface < equilibrium.radialSurfaces; ++surface)
        {
            const double s = static_cast<double>(surface) / (equilibrium.radialSurfaces - 1);
            const size_t offset = static_cast<size_t>(surface) * equilibrium.modes.size();
            equilibrium.radiusCos[offset] = 5.0;
            equilibrium.radiusCos[offset + 1] = std::sqrt(s);
            equilibrium.radiusCos[offset + 2] = 0.03 * s * s;
            equilibrium.heightSin[offset + 1] = std::sqrt(s);
            equilibrium.heightSin[offset + 2] = 0.02 * s * s;
        }
        for (int surface = 0; surface < equilibrium.radialSurfaces; ++surface)
        {
            const double s = std::max(0.0, (surface - 0.5) / (equilibrium.radialSurfaces - 1));
            const size_t offset = static_cast<size_t>(surface) * equilibrium.fieldModes.size();
            equilibrium.bThetaCos[offset + 1] = 0.04 * std::sqrt(s);
            equilibrium.bPhiCos[offset] = 1.0 + 0.2 * s;
            equilibrium.bPhiCos[offset + 2] = 0.1 * s * s;
            equilibrium.bStrengthCos[offset] = 1.0 + 0.2 * s;
        }
        return equilibrium;
    }

    std::array<double, 3> toCartesian(const std::array<double, 3>& cylindrical, const double phi)
    {
        return {cylindrical[0] * std::cos(phi) - cylindrical[1] * std::sin(phi), cylindrical[0] * std::sin(phi) + cylindrical[1] * std::cos(phi), cylindrical[2]};
    }
} // namespace

TEST(VmecFieldProviderTest, DirectEvaluationUsesVmecContravariantComponents)
{
    const auto equilibrium = circularEquilibrium();

    const auto point = simulation::evaluateVmecField(equilibrium, 0.4, 0.7, 0.3);

    EXPECT_NEAR(point.radius, 5.0 + 0.4 * std::cos(0.7), 1e-12);
    EXPECT_NEAR(point.height, 0.4 * std::sin(0.7), 1e-12);
    EXPECT_NEAR(point.bCylindrical[0], 0.0, 1e-12);
    EXPECT_NEAR(point.bCylindrical[1], point.radius, 1e-12);
    EXPECT_NEAR(point.bCylindrical[2], 0.0, 1e-12);
    EXPECT_NEAR(point.bStrength, 1.0, 1e-12);
}

TEST(VmecFieldProviderTest, InterpolatedFieldMatchesDirectFourierEvaluation)
{
    const auto equilibrium = circularEquilibrium();
    const simulation::VmecFieldProvider field(equilibrium, {.radial = 24, .poloidal = 48, .toroidal = 12});

    for (int radialIndex = 1; radialIndex <= 9; ++radialIndex)
    {
        const double rho = radialIndex / 10.0;
        const double theta = 0.19 + radialIndex * 0.51;
        const double phi = 0.13 + radialIndex * 0.17;
        const auto position = field.position({rho, theta, phi});
        const auto actual = field.tryB(position);
        ASSERT_TRUE(actual.has_value());
        const auto actualValue = actual.value_or(std::array<double, 3>{});

        const auto reference = simulation::evaluateVmecField(equilibrium, rho, theta, phi);
        const auto expected = toCartesian(reference.bCylindrical, phi);
        for (size_t component = 0; component < expected.size(); ++component)
        {
            EXPECT_NEAR(actualValue[component], expected[component], 5e-6);
        }

        const auto coordinates = field.locate(position);
        ASSERT_TRUE(coordinates.has_value());
        const auto coordinateValue = coordinates.value_or(simulation::FluxCoordinates{});
        EXPECT_NEAR(coordinateValue.rho, rho, 1e-8);
        EXPECT_NEAR(std::remainder(coordinateValue.theta - theta, 2.0 * std::numbers::pi), 0.0, 1e-8);
        EXPECT_NEAR(std::remainder(coordinateValue.phi - phi, 2.0 * std::numbers::pi), 0.0, 1e-8);
    }
}

TEST(VmecFieldProviderTest, RejectsPointsOutsideLastClosedFluxSurface)
{
    const auto equilibrium = circularEquilibrium();
    const simulation::VmecFieldProvider field(equilibrium, {.radial = 24, .poloidal = 48, .toroidal = 12});
    const auto outside = field.position({.rho = 1.2, .theta = 0.8, .phi = 0.4});

    EXPECT_FALSE(field.contains(outside));
    EXPECT_FALSE(field.tryB(outside).has_value());
}

TEST(VmecFieldProviderTest, FieldInterpolationConvergesAsGridResolutionIncreases)
{
    const auto equilibrium = varyingEquilibrium();
    const auto maximumError = [&equilibrium](const simulation::VmecGridSpec grid)
    {
        const simulation::VmecFieldProvider field(equilibrium, grid);
        double maximum = 0.0;
        for (int index = 0; index < 30; ++index)
        {
            const double rho = 0.12 + 0.025 * index;
            const double theta = 0.17 + 0.41 * index;
            const double phi = 0.09 + 0.23 * index;
            const auto position = field.position({rho, theta, phi});
            const auto actual = field.tryB(position);
            EXPECT_TRUE(actual.has_value());
            if (!actual)
            {
                continue;
            }
            const auto reference = simulation::evaluateVmecField(equilibrium, rho, theta, phi);
            const auto expected = toCartesian(reference.bCylindrical, phi);
            std::array<double, 3> difference{};
            for (size_t component = 0; component < difference.size(); ++component)
            {
                difference[component] = (*actual)[component] - expected[component];
            }
            maximum = std::max(maximum, std::sqrt(difference[0] * difference[0] + difference[1] * difference[1] + difference[2] * difference[2]));
        }
        return maximum;
    };

    const double coarseError = maximumError({.radial = 8, .poloidal = 16, .toroidal = 8});
    const double fineError = maximumError({.radial = 40, .poloidal = 96, .toroidal = 32});

    EXPECT_GT(coarseError, 0.0);
    EXPECT_LT(fineError, coarseError);
}
