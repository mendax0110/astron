#pragma once

#include <filesystem>

#include "../../third_party/mexVec/include/Vec.hpp"

/**
 * @brief Astron namespace for core data structures and algorithms \namespace astron::core
 */
namespace astron::core
{
    /**
     * @brief Represents a Fourier mode with poloidal and toroidal mode numbers. \struct FourierMode
     */
    struct FourierMode
    {
        double m = 0.0;
        double n = 0.0;
    };

    /**
     * @brief Represents a VMEC equilibrium with Fourier modes and geometric data. \struct Equilibrium
     */
    struct Equilibrium
    {
        std::filesystem::path source;
        int fieldPeriods = 0;
        int radialSurfaces = 0;
        mex::Vec<FourierMode> modes;
        mex::Vec<double> radiusCos;
        mex::Vec<double> heightSin;
        mex::Vec<double> radiusSin;
        mex::Vec<double> heightCos;
        mex::Vec<double> rotationalTransform;
        double plasmaVolume = 0.0;
        double betaTotal = 0.0;

        /**
         * @brief Computes the radius at a given surface, poloidal angle (theta), and toroidal angle (phi).
         * @param surface The index of the radial surface.
         * @param theta The poloidal angle in radians.
         * @param phi The toroidal angle in radians.
         * @return A double representing the radius at the specified surface and angles.
         */
        [[nodiscard]] double radius(int surface, double theta, double phi) const;

        /**
         * @brief Computes the height at a given surface, poloidal angle (theta), and toroidal angle (phi).
         * @param surface The index of the radial surface.
         * @param theta The poloidal angle in radians.
         * @param phi The toroidal angle in radians.
         * @return A double representing the height at the specified surface and angles.
         */
        [[nodiscard]] double height(int surface, double theta, double phi) const;
    };
} // namespace astron::core
