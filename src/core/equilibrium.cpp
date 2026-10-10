#include "core/equilibrium.h"

#include <cmath>

using namespace astron::core;

double Equilibrium::radius(const int surface, const double theta, const double phi) const
{
    double value = 0.0;
    const size_t offset = static_cast<size_t>(surface) * modes.size();
    for(size_t mode = 0; mode < modes.size(); ++mode)
    {
        const double angle = modes[mode].m * theta - modes[mode].n * phi;
        value += radiusCos[offset + mode] * std::cos(angle);
        if(!radiusSin.empty())
        {
            value += radiusSin[offset + mode] * std::sin(angle);
        }
    }
    return value;
}

double Equilibrium::height(const int surface, const double theta, const double phi) const
{
    double value = 0.0;
    const size_t offset = static_cast<size_t>(surface) * modes.size();
    for(size_t mode = 0; mode < modes.size(); ++mode)
    {
        const double angle = modes[mode].m * theta - modes[mode].n * phi;
        value += heightSin[offset + mode] * std::sin(angle);
        if(!heightCos.empty())
        {
            value += heightCos[offset + mode] * std::cos(angle);
        }
    }
    return value;
}
