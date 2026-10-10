#pragma once

#include <array>
#include <cmath>

/**
 * @brief Astron namespace for particle simulation \namespace astron::simulation
 */
namespace astron::simulation
{
    /**
     * @brief Abstract base class for providing electromagnetic fields. \struct FieldProvider
     * @details This class defines the interface for obtaining magnetic (B) and electric (E) fields at a given position in space.
     */
    struct FieldProvider
    {
        /**
         * @brief Virtual destructor to allow proper cleanup of derived classes.
         */
        virtual ~FieldProvider() = default;

        /**
         * @brief Pure virtual function to obtain the magnetic field at a given position.
         * @param x A 3D position in space represented as an array of doubles.
         * @return A 3D magnetic field vector at the specified position.
         */
        virtual std::array<double, 3> B(const std::array<double, 3>& /*x*/) const = 0;

        /**
         * @brief Pure virtual function to obtain the electric field at a given position.
         * @param x A 3D position in space represented as an array of doubles.
         * @return A 3D electric field vector at the specified position.
         */
        virtual std::array<double, 3> E(const std::array<double, 3>& /*x*/) const
        {
            return {0.0, 0.0, 0.0};
        }
    };

    /**
     * @brief Simple uniform magnetic field provider. \struct UniformField
     * @details This class provides a uniform magnetic field in the z-direction with a specified magnitude.
     */
    struct UniformField : FieldProvider
    {
        /**
         * @brief Constructs a UniformField with a specified magnetic field strength in the z-direction.
         * @param bz The magnitude of the uniform magnetic field in the z-direction.
         */
        explicit UniformField(const double bz) : bz_(bz)
        {
        }

        /**
         * @brief Returns the uniform magnetic field vector at any position.
         * @param x A 3D position in space (not used in this implementation).
         * @return A 3D magnetic field vector with the specified z-component.
         */
        std::array<double, 3> B(const std::array<double, 3>&) const override
        {
            return {0.0, 0.0, bz_};
        }

        double bz_ = 0.0;
    };

    /**
     * @brief Helical magnetic field provider. \struct HelicalField
     * @details This class provides a helical magnetic field with specified parameters for the helical and axial components.
     * Simple helical/demo field: B = (B0*sin(k*z), B0*cos(k*z), Bz)
     */
    struct HelicalField : FieldProvider
    {
        /**
         * @brief Constructs a HelicalField with specified parameters for the helical and axial components.
         * @param b0 The magnitude of the helical component of the magnetic field.
         * @param bz The magnitude of the axial component of the magnetic field.
         * @param k The wave number for the helical component, determining its spatial frequency along the z-axis.
         */
        HelicalField(const double b0, const double bz, const double k) : b0_(b0), bz_(bz), k_(k)
        {
        }

        /**
         * @brief Returns the helical magnetic field vector at a given position.
         * @param x A 3D position in space, where x[2] is used to compute the helical component.
         * @return A 3D magnetic field vector with helical and axial components based on the specified parameters.
         */
        std::array<double, 3> B(const std::array<double, 3>& x) const override
        {
            const double s = std::sin(k_ * x[2]);
            const double c = std::cos(k_ * x[2]);
            return {b0_ * s, b0_ * c, bz_};
        }

        double b0_ = 0.0;
        double bz_ = 0.0;
        double k_ = 1.0;
    };
} // namespace astron::simulation
