#include "core/equilibrium.h"

#include <gtest/gtest.h>

#include <numbers>

using namespace astron;

TEST(EquilibriumTest, EvaluatesFourierModesAndRadialOffset)
{
    core::Equilibrium equilibrium;
    equilibrium.radialSurfaces = 2;
    equilibrium.modes = {{0.0, 0.0}, {1.0, 0.0}};
    equilibrium.radiusCos = {5.0, 0.5, 5.2, 0.7};
    equilibrium.radiusSin = {0.0, 0.1, 0.0, 0.2};
    equilibrium.heightSin = {0.0, 0.5, 0.0, 0.7};
    equilibrium.heightCos = {0.0, 0.1, 0.0, 0.2};

    EXPECT_NEAR(equilibrium.radius(0, 0.0, 0.0), 5.5, 1e-12);
    EXPECT_NEAR(equilibrium.radius(1, 0.0, 0.0), 5.9, 1e-12);
    EXPECT_NEAR(equilibrium.radius(1, std::numbers::pi / 2.0, 0.0), 5.4, 1e-12);
    EXPECT_NEAR(equilibrium.height(1, 0.0, 0.0), 0.2, 1e-12);
    EXPECT_NEAR(equilibrium.height(1, std::numbers::pi / 2.0, 0.0), 0.7, 1e-12);
}

TEST(EquilibriumTest, AppliesToroidalModeNumber)
{
    core::Equilibrium equilibrium;
    equilibrium.modes = {{0.0, 5.0}};
    equilibrium.radiusCos = {1.0};
    equilibrium.heightSin = {1.0};

    EXPECT_NEAR(equilibrium.radius(0, 0.0, std::numbers::pi / 5.0), -1.0, 1e-12);
    EXPECT_NEAR(equilibrium.height(0, 0.0, std::numbers::pi / 10.0), -1.0, 1e-12);
}
