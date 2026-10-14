#include "visualization/stellarator_geometry.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>

#include <cmath>
#include <numbers>

using namespace astron::visualization;

namespace
{
    constexpr double Pi = std::numbers::pi;
    constexpr int PoloidalSamples = 64;
    constexpr int ToroidalSamples = 160;
} // namespace

RGBColorValues astron::visualization::colorValues(const ColorScheme scheme)
{
    switch (scheme)
    {
    case ColorScheme::Plasma:
        return {0.17, 0.72, 0.84, 0.58};
    case ColorScheme::Coils:
        return {0.90, 0.72, 0.42, 0.78};
    case ColorScheme::GuideField:
        return {0.56, 0.89, 0.76, 1.0};
    case ColorScheme::Marker:
        return {0.82, 0.96, 1.0, 1.0};
    case ColorScheme::Renderer:
        return {1.0, 0.2, 0.2, 1.0};
    default:
        return {1.0, 1.0, 1.0, 1.0}; // Default to white if unknown
    }
}

std::array<double, 3> astron::visualization::position(const core::Equilibrium* equilibrium, const int surface, const double theta, const double phi, const double scale)
{
    double radius = 0.0;
    double height = 0.0;
    if (equilibrium != nullptr)
    {
        radius = equilibrium->radius(surface, theta, phi);
        height = equilibrium->height(surface, theta, phi);
    }
    else
    {
        radius = 5.5 + scale * (0.55 * std::cos(theta) + 0.08 * std::cos(theta - 5.0 * phi));
        height = scale * (0.57 * std::sin(theta) + 0.08 * std::sin(theta - 5.0 * phi));
    }
    return {radius * std::cos(phi), radius * std::sin(phi), height};
}

vtkSmartPointer<vtkPolyData> astron::visualization::surfaceData(const core::Equilibrium* equilibrium, const int surface, const double scale)
{
    const auto points = vtkSmartPointer<vtkPoints>::New();
    const auto faces = vtkSmartPointer<vtkCellArray>::New();
    for (int toroidal = 0; toroidal < ToroidalSamples; ++toroidal)
    {
        const double phi = 2.0 * Pi * toroidal / ToroidalSamples;
        for (int poloidal = 0; poloidal < PoloidalSamples; ++poloidal)
        {
            const double theta = 2.0 * Pi * poloidal / PoloidalSamples;
            const auto point = position(equilibrium, surface, theta, phi, scale);
            points->InsertNextPoint(point.data());
        }
    }
    for (int toroidal = 0; toroidal < ToroidalSamples; ++toroidal)
    {
        for (int poloidal = 0; poloidal < PoloidalSamples; ++poloidal)
        {
            const int nextToroidal = (toroidal + 1) % ToroidalSamples;
            const int nextPoloidal = (poloidal + 1) % PoloidalSamples;
            std::array<vtkIdType, 4> corners = {static_cast<vtkIdType>(toroidal * PoloidalSamples + poloidal), static_cast<vtkIdType>(nextToroidal * PoloidalSamples + poloidal),
                                                static_cast<vtkIdType>(nextToroidal * PoloidalSamples + nextPoloidal),
                                                static_cast<vtkIdType>(toroidal * PoloidalSamples + nextPoloidal)};
            faces->InsertNextCell(4, corners.data());
        }
    }
    auto data = vtkSmartPointer<vtkPolyData>::New();
    data->SetPoints(points);
    data->SetPolys(faces);
    return data;
}

vtkSmartPointer<vtkPolyData> astron::visualization::schematicCoils()
{
    const auto points = vtkSmartPointer<vtkPoints>::New();
    const auto lines = vtkSmartPointer<vtkCellArray>::New();
    for (int coil = 0; coil < 50; ++coil)
    {
        const double center = 2.0 * Pi * coil / 50.0;
        lines->InsertNextCell(97);
        for (int step = 0; step <= 96; ++step)
        {
            const double theta = 2.0 * Pi * step / 96.0;
            const double phi = center + 0.045 * std::sin(theta + 5.0 * center);
            const double radius = 5.5 + 0.93 * std::cos(theta) + 0.13 * std::cos(theta - 5.0 * center);
            const double height = 0.95 * std::sin(theta) + 0.13 * std::sin(theta - 5.0 * center);
            const auto id = points->InsertNextPoint(radius * std::cos(phi), radius * std::sin(phi), height);
            lines->InsertCellPoint(id);
        }
    }
    const auto centerlines = vtkSmartPointer<vtkPolyData>::New();
    centerlines->SetPoints(points);
    centerlines->SetLines(lines);
    const auto tubes = vtkSmartPointer<vtkTubeFilter>::New();
    tubes->SetInputData(centerlines);
    tubes->SetRadius(0.025);
    tubes->SetNumberOfSides(8);
    tubes->Update();
    auto result = vtkSmartPointer<vtkPolyData>::New();
    result->ShallowCopy(tubes->GetOutput());
    return result;
}

vtkSmartPointer<vtkActor> astron::visualization::actorFor(vtkPolyData* data, const RGBColorValues& color)
{
    const auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(data);
    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.r, color.g, color.b);
    actor->GetProperty()->SetOpacity(color.opacity);
    return actor;
}
