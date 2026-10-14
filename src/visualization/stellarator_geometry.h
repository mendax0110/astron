#pragma once

#include "core/equilibrium.h"
#include <array>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkPolyData;

/**
 * @brief Astron namespace for visualization utilities \namespace astron::visualization
 */
namespace astron::visualization
{
    /**
     * @brief Enum representing different color schemes for visualization. \enum ColorScheme
     */
    enum class ColorScheme
    {
        Plasma = 0,
        Coils = 1,
        GuideField = 2,
        Marker = 3,
        Renderer = 4
    };

    /**
     * @brief Struct to hold RGB color values and opacity for visualization. \struct RGBColorValues
     */
    struct RGBColorValues
    {
        double r;
        double g;
        double b;
        double opacity;
    };

    /**
     * @brief Returns the RGB color values and opacity for a given color scheme.
     * @param scheme The ColorScheme enum value specifying the desired color scheme.
     * @return An RGBColorValues struct containing the red, green, blue, and opacity values for the specified color scheme.
     */
    [[nodiscard]] RGBColorValues colorValues(ColorScheme scheme);

    /**
     * @brief Computes the 3D position of a point on a stellarator surface based on the equilibrium data, surface index, poloidal angle (theta), toroidal angle (phi), and scale
     * factor.
     * @param equilibrium Pointer to the Equilibrium object containing the stellarator geometry data. Can be nullptr for default geometry.
     * @param surface The index of the radial surface to evaluate.
     * @param theta The poloidal angle in radians.
     * @param phi The toroidal angle in radians.
     * @param scale A scaling factor applied to the computed position.
     * @return An array of three doubles representing the 3D coordinates (x, y, z) of the point on the surface.
     */
    [[nodiscard]] std::array<double, 3> position(const core::Equilibrium* equilibrium, int surface, double theta, double phi, double scale);

    /**
     * @brief Generates a vtkPolyData object representing the surface geometry of a stellarator based on the equilibrium data, surface index, and scale factor.
     * @param equilibrium Pointer to the Equilibrium object containing the stellarator geometry data. Can be nullptr for default geometry.
     * @param surface The index of the radial surface to generate.
     * @param scale A scaling factor applied to the generated surface geometry.
     * @return A vtkSmartPointer to a vtkPolyData object containing the surface geometry.
     */
    [[nodiscard]] vtkSmartPointer<vtkPolyData> surfaceData(const core::Equilibrium* equilibrium, int surface, double scale);

    /**
     * @brief Generates a vtkPolyData object representing the schematic coils of a stellarator.
     * @return A vtkSmartPointer to a vtkPolyData object containing the schematic coil geometry.
     */
    [[nodiscard]] vtkSmartPointer<vtkPolyData> schematicCoils();

    /**
     * @brief Creates a vtkActor object for rendering a given vtkPolyData geometry with specified color and opacity.
     * @param data Pointer to the vtkPolyData object containing the geometry.
     * @param color An RGBColorValues struct specifying the color and opacity for the actor.
     * @return A vtkSmartPointer to a vtkActor object.
     */
    [[nodiscard]] vtkSmartPointer<vtkActor> actorFor(vtkPolyData* data, const RGBColorValues& color);
} // namespace astron::visualization
