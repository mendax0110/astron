#include "simulation/vmec_field_provider.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

using namespace astron::simulation;

namespace
{
    constexpr double TwoPi = 2.0 * std::numbers::pi;
    constexpr int ChannelCount = 5; // R, Z, B_R, B_phi, B_Z
    constexpr double NewtonTolerance = 1e-10;
    constexpr int NewtonIterations = 60;
    constexpr double InsideTolerance = 1e-9;
    constexpr double MaxRho = 1.5;

    /**
     * @brief Checks if the mode is odd and computes the coefficient using a square root interpolation if true, otherwise uses linear interpolation.
     * @param m The poloidal mode number of the current Fourier mode.
     * @return true if the mode is odd, false otherwise.
     */
    bool isOdd(const double m)
    {
        return std::fmod(std::abs(m), 2.0) > 0.5;
    }

    /**
     * @brief Coefficients of every mode at normalized flux s, blended between VMEC mesh rows.
     * @param data The data vector containing the coefficients for each mode at each surface.
     * @param modes The vector of Fourier modes corresponding to the coefficients in the data vector.
     * @param surfaces The number of radial surfaces in the equilibrium.
     * @param halfMesh True if the data is defined on a half-mesh (i.e., the first surface is at s=0.5*ds), false if it is defined on a full mesh (i.e., the first surface is at
     * s=0).
     * @param s The normalized flux value at which to evaluate the coefficients.
     * @return The evaluated coefficients for each mode.
     */
    mex::Vec<double> coefficientsAt(const mex::Vec<double>& data, const mex::Vec<astron::core::FourierMode>& modes, const int surfaces, const bool halfMesh, const double s)
    {
        const size_t count = modes.size();
        const double ds = 1.0 / (surfaces - 1);
        const double t = halfMesh ? s / ds + 0.5 : s / ds;
        const int last = surfaces - 1;
        mex::Vec<double> result(count);
        for (size_t mode = 0; mode < count; ++mode)
        {
            const bool odd = isOdd(modes[mode].m);
            const int first = (halfMesh || odd) ? 1 : 0;
            const int lower = std::clamp(static_cast<int>(std::floor(t)), first, last - 1);
            const double weight = t - lower;
            const double sLow = halfMesh ? (lower - 0.5) * ds : lower * ds;
            const double sHigh = sLow + ds;
            const double low = data[static_cast<size_t>(lower) * count + mode];
            const double high = data[static_cast<size_t>(lower + 1) * count + mode];
            if (odd)
            {
                result[mode] = std::sqrt(std::max(s, 0.0)) * ((1.0 - weight) * low / std::sqrt(sLow) + weight * high / std::sqrt(sHigh));
            }
            else
            {
                result[mode] = (1.0 - weight) * low + weight * high;
            }
        }
        return result;
    }

    /**
     * @brief Struct representing the weights and indices for periodic interpolation in a circular domain. \struct Weights
     */
    struct Weights
    {
        std::array<int, 4> index{};
        std::array<double, 4> value{};
        std::array<double, 4> slope{}; // d/dtau
    };

    /**
     * @brief Four-point Lagrange weights at nodes tau = -1, 0, 1, 2.
     * @param tau The normalized distance from the lower node (0 <= tau <= 1).
     * @param w The Weights struct to store the computed weights, indices, and slopes.
     */
    void lagrange(const double tau, Weights& w)
    {
        w.value = {-tau * (tau - 1.0) * (tau - 2.0) / 6.0, (tau + 1.0) * (tau - 1.0) * (tau - 2.0) / 2.0, -(tau + 1.0) * tau * (tau - 2.0) / 2.0,
                   (tau + 1.0) * tau * (tau - 1.0) / 6.0};
        w.slope = {-(3.0 * tau * tau - 6.0 * tau + 2.0) / 6.0, (3.0 * tau * tau - 4.0 * tau - 1.0) / 2.0, -(3.0 * tau * tau - 2.0 * tau - 2.0) / 2.0,
                   (3.0 * tau * tau - 1.0) / 6.0};
    }

    /**
     * @brief Computes the periodic weights for interpolation in a circular domain, wrapping around the indices as necessary.
     * @param angle The angle in radians for which to compute the periodic weights.
     * @param period The period of the circular domain (e.g., 2*pi for a full circle).
     * @param count The number of discrete points in the circular domain.
     * @return A Weights struct containing the computed weights, indices, and slopes for interpolation.
     */
    Weights periodicWeights(const double angle, const double period, const int count)
    {
        double x = std::fmod(angle, period);
        if (x < 0.0)
        {
            x += period;
        }
        const double t = x / (period / count);
        const int cell = static_cast<int>(std::floor(t));
        Weights w;
        lagrange(t - cell, w);
        for (int q = 0; q < 4; ++q)
        {
            w.index[q] = ((cell - 1 + q) % count + count) % count;
        }
        return w;
    }

    /**
     * @brief Radial Weights for interpolation in a structured grid, ensuring that the indices are clamped within valid bounds.
     * @param rho The radial coordinate (normalized flux) for which to compute the weights.
     * @param h The grid spacing in the radial direction.
     * @param count The number of discrete points in the radial direction.
     * @return A Weights struct containing the computed weights, indices, and slopes for interpolation.
     */
    Weights radialWeights(const double rho, const double h, const int count)
    {
        const double t = rho / h;
        const int start = std::clamp(static_cast<int>(std::floor(t)) - 1, 0, count - 4);
        Weights w;
        lagrange(t - (start + 1), w);
        for (int q = 0; q < 4; ++q)
        {
            w.index[q] = start + q;
        }
        return w;
    }

    /**
     * @brief Wraps the angle to the range [0, 2*pi) to ensure periodicity in the evaluation of the VMEC field.
     * @param angle The angle to be wrapped, in radians.
     * @return The wrapped angle in the range [0, 2*pi).
     */
    double wrapTwoPi(const double angle)
    {
        double x = std::fmod(angle, TwoPi);
        if (x < 0.0)
        {
            x += TwoPi;
        }
        return x;
    }
} // namespace

VmecPoint astron::simulation::evaluateVmecField(const core::Equilibrium& equilibrium, const double rho, const double theta, const double phi)
{
    if (!equilibrium.hasField() || equilibrium.radialSurfaces < 3)
    {
        throw std::invalid_argument("Equilibrium has no usable field data");
    }
    const int ns = equilibrium.radialSurfaces;
    const double s = rho * rho;
    const auto rc = coefficientsAt(equilibrium.radiusCos, equilibrium.modes, ns, false, s);
    const auto zc = coefficientsAt(equilibrium.heightSin, equilibrium.modes, ns, false, s);
    const auto bu = coefficientsAt(equilibrium.bThetaCos, equilibrium.fieldModes, ns, true, s);
    const auto bv = coefficientsAt(equilibrium.bPhiCos, equilibrium.fieldModes, ns, true, s);

    double r = 0.0, z = 0.0, rTheta = 0.0, rPhi = 0.0, zTheta = 0.0, zPhi = 0.0;
    for (size_t mode = 0; mode < equilibrium.modes.size(); ++mode)
    {
        const double m = equilibrium.modes[mode].m;
        const double n = equilibrium.modes[mode].n;
        const double angle = m * theta - n * phi;
        const double c = std::cos(angle);
        const double sn = std::sin(angle);
        r += rc[mode] * c;
        rTheta -= rc[mode] * m * sn;
        rPhi += rc[mode] * n * sn;
        z += zc[mode] * sn;
        zTheta += zc[mode] * m * c;
        zPhi -= zc[mode] * n * c;
    }
    double bTheta = 0.0, bPhi = 0.0, strength = 0.0;
    for (size_t mode = 0; mode < equilibrium.fieldModes.size(); ++mode)
    {
        const double c = std::cos(equilibrium.fieldModes[mode].m * theta - equilibrium.fieldModes[mode].n * phi);
        bTheta += bu[mode] * c;
        bPhi += bv[mode] * c;
    }
    if (!equilibrium.bStrengthCos.empty())
    {
        const auto bm = coefficientsAt(equilibrium.bStrengthCos, equilibrium.fieldModes, ns, true, s);
        for (size_t mode = 0; mode < equilibrium.fieldModes.size(); ++mode)
        {
            strength += bm[mode] * std::cos(equilibrium.fieldModes[mode].m * theta - equilibrium.fieldModes[mode].n * phi);
        }
    }

    VmecPoint point;
    point.radius = r;
    point.height = z;
    point.bCylindrical = {bTheta * rTheta + bPhi * rPhi, bPhi * r, bTheta * zTheta + bPhi * zPhi};
    point.bStrength = strength;
    return point;
}

VmecFieldProvider::VmecFieldProvider(const core::Equilibrium& equilibrium, const VmecGridSpec grid)
    : fieldPeriods_(equilibrium.fieldPeriods), radial_(grid.radial), poloidal_(grid.poloidal), toroidal_(grid.toroidal)
{
    if (!equilibrium.hasField())
    {
        throw std::invalid_argument("Equilibrium has no contravariant field data (bsupumnc/bsupvmnc)");
    }
    const int ns = equilibrium.radialSurfaces;
    const size_t geometrySize = static_cast<size_t>(ns) * equilibrium.modes.size();
    if (fieldPeriods_ < 1 || ns < 3 || equilibrium.radiusCos.size() != geometrySize || equilibrium.heightSin.size() != geometrySize)
    {
        throw std::invalid_argument("Equilibrium geometry is inconsistent");
    }
    if (!equilibrium.radiusSin.empty() || !equilibrium.heightCos.empty())
    {
        throw std::invalid_argument("Stellarator-asymmetric equilibria are not supported");
    }
    if (radial_ < 4 || poloidal_ < 8 || toroidal_ < 4)
    {
        throw std::invalid_argument("Interpolation grid is too small");
    }

    hRho_ = 1.0 / (radial_ - 1);
    hTheta_ = TwoPi / poloidal_;
    period_ = TwoPi / fieldPeriods_;
    hPhi_ = period_ / toroidal_;

    int mMax = 0;
    int kMax = 0;
    const auto toroidalIndex = [&](const double n)
    {
        const long k = std::lround(n / fieldPeriods_);
        if (std::abs(n - static_cast<double>(k) * fieldPeriods_) > 1e-9)
        {
            throw std::invalid_argument("Toroidal mode number is not a multiple of nfp");
        }
        return static_cast<int>(k);
    };
    mex::Vec<int> geometryM, geometryK, fieldM, fieldK;
    for (const auto& [m, n] : equilibrium.modes)
    {
        geometryM.push_back(static_cast<int>(std::lround(m)));
        geometryK.push_back(toroidalIndex(n));
    }
    for (const auto& [m, n] : equilibrium.fieldModes)
    {
        fieldM.push_back(static_cast<int>(std::lround(m)));
        fieldK.push_back(toroidalIndex(n));
    }
    for (const auto* list : {&geometryM, &fieldM})
    {
        for (const int m : *list)
        {
            mMax = std::max(mMax, m);
        }
    }
    for (const auto* list : {&geometryK, &fieldK})
    {
        for (const int k : *list)
        {
            kMax = std::max(kMax, std::abs(k));
        }
    }
    mex::Vec<double> cosTheta(static_cast<size_t>(mMax + 1) * poloidal_), sinTheta(cosTheta.size());
    for (int m = 0; m <= mMax; ++m)
    {
        for (int j = 0; j < poloidal_; ++j)
        {
            cosTheta[static_cast<size_t>(m) * poloidal_ + j] = std::cos(m * hTheta_ * j);
            sinTheta[static_cast<size_t>(m) * poloidal_ + j] = std::sin(m * hTheta_ * j);
        }
    }
    mex::Vec<double> cosPhi(static_cast<size_t>(2 * kMax + 1) * toroidal_), sinPhi(cosPhi.size());
    for (int k = -kMax; k <= kMax; ++k)
    {
        for (int l = 0; l < toroidal_; ++l)
        {
            // n * phi_l = k * nfp * (2 pi / nfp) * l / toroidal = 2 pi k l / toroidal
            const double angle = TwoPi * (static_cast<double>(k) * l) / toroidal_;
            cosPhi[static_cast<size_t>(k + kMax) * toroidal_ + l] = std::cos(angle);
            sinPhi[static_cast<size_t>(k + kMax) * toroidal_ + l] = std::sin(angle);
        }
    }

    nodes_.assign(static_cast<size_t>(radial_) * poloidal_ * toroidal_ * ChannelCount, 0.0);
    for (int i = 0; i < radial_; ++i)
    {
        const double rho = std::min(1.0, i * hRho_);
        const double s = rho * rho;
        const auto rc = coefficientsAt(equilibrium.radiusCos, equilibrium.modes, ns, false, s);
        const auto zc = coefficientsAt(equilibrium.heightSin, equilibrium.modes, ns, false, s);
        const auto bu = coefficientsAt(equilibrium.bThetaCos, equilibrium.fieldModes, ns, true, s);
        const auto bv = coefficientsAt(equilibrium.bPhiCos, equilibrium.fieldModes, ns, true, s);
        for (int j = 0; j < poloidal_; ++j)
        {
            for (int l = 0; l < toroidal_; ++l)
            {
                double r = 0.0, z = 0.0, rTheta = 0.0, rPhi = 0.0, zTheta = 0.0, zPhi = 0.0;
                for (size_t mode = 0; mode < rc.size(); ++mode)
                {
                    const size_t mi = static_cast<size_t>(geometryM[mode]) * poloidal_ + j;
                    const size_t ni = static_cast<size_t>(geometryK[mode] + kMax) * toroidal_ + l;
                    const double c = cosTheta[mi] * cosPhi[ni] + sinTheta[mi] * sinPhi[ni];  // cos(m theta - n phi)
                    const double sn = sinTheta[mi] * cosPhi[ni] - cosTheta[mi] * sinPhi[ni]; // sin(m theta - n phi)
                    const double m = equilibrium.modes[mode].m;
                    const double n = equilibrium.modes[mode].n;
                    r += rc[mode] * c;
                    rTheta -= rc[mode] * m * sn;
                    rPhi += rc[mode] * n * sn;
                    z += zc[mode] * sn;
                    zTheta += zc[mode] * m * c;
                    zPhi -= zc[mode] * n * c;
                }
                double bTheta = 0.0, bPhi = 0.0;
                for (size_t mode = 0; mode < bu.size(); ++mode)
                {
                    const size_t mi = static_cast<size_t>(fieldM[mode]) * poloidal_ + j;
                    const size_t ni = static_cast<size_t>(fieldK[mode] + kMax) * toroidal_ + l;
                    const double c = cosTheta[mi] * cosPhi[ni] + sinTheta[mi] * sinPhi[ni];
                    bTheta += bu[mode] * c;
                    bPhi += bv[mode] * c;
                }
                double* node = &nodes_[((static_cast<size_t>(i) * poloidal_ + j) * toroidal_ + l) * ChannelCount];
                node[0] = r;
                node[1] = z;
                node[2] = bTheta * rTheta + bPhi * rPhi; // B_R
                node[3] = bPhi * r;                      // B_phi
                node[4] = bTheta * zTheta + bPhi * zPhi; // B_Z
            }
        }
    }
}

void VmecFieldProvider::sample(const double rho, const double theta, const double phi, const int firstChannel, const int channelCount, const bool derivatives, Sample& out) const
{
    const auto [indexRw, valueRw, slopeRw] = radialWeights(rho, hRho_, radial_);
    const auto [indexPw, valuePw, slopePw] = periodicWeights(theta, TwoPi, poloidal_);
    const Weights wp = periodicWeights(phi, period_, toroidal_);
    const double scaleRho = 1.0 / hRho_;
    const double scaleTheta = 1.0 / hTheta_;
    for (int c = firstChannel; c < firstChannel + channelCount; ++c)
    {
        out.value[c] = 0.0;
        out.dRho[c] = 0.0;
        out.dTheta[c] = 0.0;
    }
    for (int a = 0; a < 4; ++a)
    {
        for (int b = 0; b < 4; ++b)
        {
            for (int d = 0; d < 4; ++d)
            {
                const double* node = &nodes_[((static_cast<size_t>(indexRw[a]) * poloidal_ + indexPw[b]) * toroidal_ + wp.index[d]) * ChannelCount];
                const double value = valueRw[a] * valuePw[b] * wp.value[d];
                const double dRho = derivatives ? slopeRw[a] * scaleRho * valuePw[b] * wp.value[d] : 0.0;
                const double dTheta = derivatives ? valueRw[a] * slopePw[b] * scaleTheta * wp.value[d] : 0.0;
                for (int c = firstChannel; c < firstChannel + channelCount; ++c)
                {
                    out.value[c] += value * node[c];
                    out.dRho[c] += dRho * node[c];
                    out.dTheta[c] += dTheta * node[c];
                }
            }
        }
    }
}

std::optional<FluxCoordinates> VmecFieldProvider::locate(const std::array<double, 3>& x) const
{
    const double r = std::hypot(x[0], x[1]);
    const double z = x[2];
    const double phi = std::atan2(x[1], x[0]);

    // give a robust Newton start (nearest node)
    double wrapped = std::fmod(phi, period_);
    if (wrapped < 0.0)
    {
        wrapped += period_;
    }
    const int plane = static_cast<int>(std::lround(wrapped / hPhi_)) % toroidal_;
    double best = 1e300;
    int bestI = 0;
    int bestJ = 0;
    for (int i = 0; i < radial_; i += 2)
    {
        for (int j = 0; j < poloidal_; j += 4)
        {
            const double* node = &nodes_[((static_cast<size_t>(i) * poloidal_ + j) * toroidal_ + plane) * ChannelCount];
            const double distance = (node[0] - r) * (node[0] - r) + (node[1] - z) * (node[1] - z);
            if (distance < best)
            {
                best = distance;
                bestI = i;
                bestJ = j;
            }
        }
    }
    double rho = std::max(bestI * hRho_, 0.05);
    double theta = bestJ * hTheta_;

    bool converged = false;
    Sample s;
    for (int iteration = 0; iteration < NewtonIterations; ++iteration)
    {
        sample(rho, theta, phi, 0, 2, true, s);
        const double fR = s.value[0] - r;
        const double fZ = s.value[1] - z;
        if (std::hypot(fR, fZ) < NewtonTolerance)
        {
            converged = true;
            break;
        }
        const double a = s.dRho[0];
        const double b = s.dTheta[0];
        const double c = s.dRho[1];
        const double d = s.dTheta[1];
        double det = a * d - b * c;
        if (std::abs(det) < 1e-12)
        {
            det = det < 0.0 ? -1e-12 : 1e-12;
        }
        const double stepRho = std::clamp(-(d * fR - b * fZ) / det, -0.3, 0.3);
        const double stepTheta = std::clamp(-(-c * fR + a * fZ) / det, -0.6, 0.6);
        rho += stepRho;
        theta += stepTheta;
        if (rho < 0.0)
        {
            rho = -rho; // passed through the magnetic axis
            theta += std::numbers::pi;
        }
        if (rho > MaxRho)
        {
            return std::nullopt;
        }
    }
    if (!converged)
    {
        return std::nullopt;
    }
    return FluxCoordinates{rho, wrapTwoPi(theta), phi};
}

bool VmecFieldProvider::contains(const std::array<double, 3>& x) const
{
    const auto c = locate(x);
    return c.has_value() && c->rho <= 1.0 + InsideTolerance;
}

std::optional<std::array<double, 3>> VmecFieldProvider::tryB(const std::array<double, 3>& x) const
{
    const auto c = locate(x);
    if (!c.has_value() || c->rho > 1.0 + InsideTolerance)
    {
        return std::nullopt;
    }
    Sample s;
    sample(c->rho, c->theta, c->phi, 2, 3, false, s);
    const double cosPhi = std::cos(c->phi);
    const double sinPhi = std::sin(c->phi);
    return std::array<double, 3>{s.value[2] * cosPhi - s.value[3] * sinPhi, s.value[2] * sinPhi + s.value[3] * cosPhi, s.value[4]};
}

std::array<double, 3> VmecFieldProvider::B(const std::array<double, 3>& x) const
{
    return tryB(x).value_or(std::array<double, 3>{0.0, 0.0, 0.0});
}

std::array<double, 3> VmecFieldProvider::position(const FluxCoordinates& c) const
{
    Sample s;
    sample(c.rho, c.theta, c.phi, 0, 2, false, s);
    return {s.value[0] * std::cos(c.phi), s.value[0] * std::sin(c.phi), s.value[1]};
}