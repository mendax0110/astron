#pragma once

#include "core/equilibrium.h"
#include "simulation/field_provider.h"

#include <array>
#include <optional>
#include <vector>

/**
 * @brief Astron namespace for particle simulation \namespace astron::simulation
 */
namespace astron::simulation
{
    /**
     * @brief Resolution of the structured (rho, theta, phi) interpolation grid. \struct VmecGridSpec
     * @details The toroidal direction covers one field period only; the field is periodic in 2*pi/nfp.
     */
    struct VmecGridSpec
    {
        int radial = 40;
        int poloidal = 96;
        int toroidal = 48;
    };

    /**
     * @brief VMEC flux coordinates. \struct FluxCoordinates
     * @details rho = sqrt(s) with s the normalized toroidal flux, theta the VMEC poloidal angle, phi the cylindrical toroidal angle.
     */
    struct FluxCoordinates
    {
        double rho = 0.0;
        double theta = 0.0;
        double phi = 0.0;
    };

    /**
     * @brief Directly summed (non-interpolated) equilibrium quantities at one flux-space point. \struct VmecPoint
     */
    struct VmecPoint
    {
        double radius = 0.0;
        double height = 0.0;
        std::array<double, 3> bCylindrical{{0.0, 0.0, 0.0}}; // (B_R, B_phi, B_Z) in tesla
        double bStrength = 0.0;                              // |B| from the bmnc series, tesla
    };

    /**
     * @brief Reference evaluation of the equilibrium by summing the Fourier series.
     * @details B = B^theta * dr/dtheta + B^phi * dr/dphi, with B^theta, B^phi from bsupumnc, bsupvmnc. Radial values blend VMEC mesh rows linearly in s; odd-m coefficients are blended as
     * c / sqrt(s) to keep the correct rho^m behaviour at the axis. This is slow and intended for grid construction and validation.
     * @param equilibrium Equilibrium with hasField() == true.
     * @param rho sqrt(s), 0 <= rho <= 1.
     * @param theta Poloidal angle in radians.
     * @param phi Toroidal angle in radians.
     * @return The summed point quantities.
     */
    [[nodiscard]] VmecPoint evaluateVmecField(const core::Equilibrium& equilibrium, double rho, double theta, double phi);

    /**
     * @brief Magnetic field of a stellarator-symmetric VMEC equilibrium, sampled from a tricubic grid. \class VmecFieldProvider
     * @details Positions are Cartesian meters with phi = atan2(y, x), matching VMEC's cylindrical frame. B is returned in tesla. E is zero (VMEC has no electrostatic potential).
     * The grid stores R, Z and (B_R, B_phi, B_Z) on (rho, theta, phi) nodes. Evaluation inverts (R, Z) -> (rho, theta) by Newton iteration on the interpolated geometry, then interpolates B.
     * Interpolation is 4-point Lagrange in each direction (fourth-order accurate values), periodic in theta and phi.
     * The provider is immutable after construction and therefore safe to call from several threads.
     */
    class VmecFieldProvider final : public FieldProvider
    {
    public:
        /**
         * @brief Builds the interpolation grid.
         * @param equilibrium Equilibrium with field data, at least three surfaces and stellarator symmetry.
         * @param grid Grid resolution.
         * @throws std::invalid_argument if the equilibrium lacks field data, is asymmetric, or the grid is too small.
         */
        explicit VmecFieldProvider(const core::Equilibrium& equilibrium, VmecGridSpec grid = {});

        /**
         * @brief Magnetic field at a Cartesian position.
         * @param x Position in meters.
         * @return B in tesla, or (0, 0, 0) if x is outside the last closed flux surface. Use tryB() to tell the two apart.
         */
        [[nodiscard]] std::array<double, 3> B(const std::array<double, 3>& x) const override;

        /**
         * @brief Magnetic field at a Cartesian position, or nothing outside the plasma.
         * @param x Position in meters.
         * @return B in tesla if x is inside the last closed flux surface.
         */
        [[nodiscard]] std::optional<std::array<double, 3>> tryB(const std::array<double, 3>& x) const;

        /**
         * @brief Finds the flux coordinates of a Cartesian position.
         * @param x Position in meters.
         * @return Coordinates (rho may slightly exceed 1 for points just outside), or nothing if the inversion did not converge or x is far outside.
         */
        [[nodiscard]] std::optional<FluxCoordinates> locate(const std::array<double, 3>& x) const;

        /**
         * @brief Checks whether a position lies inside the last closed flux surface.
         * @param x Position in meters.
         * @return true if inside.
         */
        [[nodiscard]] bool contains(const std::array<double, 3>& x) const;

        /**
         * @brief Converts flux coordinates to Cartesian position using the interpolated geometry.
         * @param c Flux coordinates.
         * @return Position in meters.
         */
        [[nodiscard]] std::array<double, 3> position(const FluxCoordinates& c) const;

    private:
        /**
         * @brief Sampled values and derivatives at a grid point. \struct Sample
         */
        struct Sample
        {
            std::array<double, 5> value{};
            std::array<double, 5> dRho{};
            std::array<double, 5> dTheta{};
        };

        /**
         * @brief Samples the grid at a given flux coordinate, optionally computing derivatives.
         * @param rho The radial coordinate (sqrt of normalized toroidal flux).
         * @param theta The poloidal coordinate.
         * @param phi The toroidal coordinate.
         * @param firstChannel The first channel to sample.
         * @param channelCount The number of channels to sample.
         * @param derivatives Whether to compute derivatives.
         * @param out The output sample.
         */
        void sample(double rho, double theta, double phi, int firstChannel, int channelCount, bool derivatives, Sample& out) const;

        int fieldPeriods_ = 0;
        int radial_ = 0;
        int poloidal_ = 0;
        int toroidal_ = 0;
        double hRho_ = 0.0;
        double hTheta_ = 0.0;
        double hPhi_ = 0.0;
        double period_ = 0.0;
        mex::Vec<double> nodes_;
    };
} // namespace astron::simulation