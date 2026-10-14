#include "boris_integrator.h"

#include "constants.h"

#include <array>
#include <cmath>

using namespace astron::simulation;

static std::array<double, 3> cross(const std::array<double, 3>& a, const std::array<double, 3>& b)
{
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

void BorisIntegrator::step(Particle& p, const double dt, const FieldProvider& provider, const bool relativistic)
{
    const auto B = provider.B(p.x);
    const auto E = provider.E(p.x);

    if (!relativistic)
    {
        const double qm = p.q / p.m;

        // v_minus = v + (q/m) * E * (dt/2)
        std::array<double, 3> v_minus = p.v;
        for (int i = 0; i < 3; ++i)
        {
            v_minus[i] += qm * E[i] * (dt * 0.5);
        }

        // t = (q/m) * B * (dt/2)
        std::array<double, 3> t{0.0, 0.0, 0.0};
        for (int i = 0; i < 3; ++i)
        {
            t[i] = qm * B[i] * (dt * 0.5);
        }

        // v_prime = v_minus + v_minus x t
        const auto v_cross_t = cross(v_minus, t);
        std::array<double, 3> v_prime;
        for (int i = 0; i < 3; ++i)
        {
            v_prime[i] = v_minus[i] + v_cross_t[i];
        }

        // s = 2t / (1 + t^2)
        const double t2 = t[0] * t[0] + t[1] * t[1] + t[2] * t[2];
        std::array<double, 3> s{0.0, 0.0, 0.0};
        const double denom = 1.0 + t2;
        if (denom != 0.0)
        {
            for (int i = 0; i < 3; ++i)
            {
                s[i] = 2.0 * t[i] / denom;
            }
        }

        // v_plus = v_minus + v_prime x s
        const auto vprime_cross_s = cross(v_prime, s);
        std::array<double, 3> v_plus;
        for (int i = 0; i < 3; ++i)
        {
            v_plus[i] = v_minus[i] + vprime_cross_s[i];
        }

        // v_new = v_plus + (q/m) * E * (dt/2)
        for (int i = 0; i < 3; ++i)
        {
            p.v[i] = v_plus[i] + qm * E[i] * (dt * 0.5);
        }

        // x_new = x + v_new * dt
        for (int i = 0; i < 3; ++i)
        {
            p.x[i] += p.v[i] * dt;
        }

        return;
    }

    // Relativistic Boris
    const double q = p.q;
    const double m = p.m;

    // u = gamma * v
    const double v2 = p.v[0] * p.v[0] + p.v[1] * p.v[1] + p.v[2] * p.v[2];
    const double gamma = 1.0 / std::sqrt(1.0 - v2 / (constants::C * constants::C));
    const std::array<double, 3> u{p.v[0] * gamma, p.v[1] * gamma, p.v[2] * gamma};

    // u_minus = u + (q/m) * E * (dt/2)
    std::array<double, 3> u_minus = u;
    for (int i = 0; i < 3; ++i)
    {
        u_minus[i] += (q / m) * E[i] * (dt * 0.5);
    }

    // gamma_minus
    const double uminus2 = u_minus[0] * u_minus[0] + u_minus[1] * u_minus[1] + u_minus[2] * u_minus[2];
    const double gamma_minus = std::sqrt(1.0 + uminus2 / (constants::C * constants::C));

    // t = (q/m) * B * (dt/2) / gamma_minus
    std::array<double, 3> t{0.0, 0.0, 0.0};
    for (int i = 0; i < 3; ++i)
    {
        t[i] = (q / m) * B[i] * (dt * 0.5) / gamma_minus;
    }

    // u_prime = u_minus + u_minus x t
    const auto uminus_cross_t = cross(u_minus, t);
    std::array<double, 3> u_prime;
    for (int i = 0; i < 3; ++i)
    {
        u_prime[i] = u_minus[i] + uminus_cross_t[i];
    }

    // s = 2t / (1 + t^2)
    const double t2 = t[0] * t[0] + t[1] * t[1] + t[2] * t[2];
    std::array<double, 3> s{0.0, 0.0, 0.0};
    const double denom = 1.0 + t2;
    if (denom != 0.0)
    {
        for (int i = 0; i < 3; ++i)
        {
            s[i] = 2.0 * t[i] / denom;
        }
    }

    // u_plus = u_minus + u_prime x s
    const auto uprime_cross_s = cross(u_prime, s);
    std::array<double, 3> u_plus;
    for (int i = 0; i < 3; ++i)
    {
        u_plus[i] = u_minus[i] + uprime_cross_s[i];
    }

    // u_new = u_plus + (q/m) * E * (dt/2)
    for (int i = 0; i < 3; ++i)
    {
        u_plus[i] += (q / m) * E[i] * (dt * 0.5);
    }

    // gamma_new
    const double u_new2 = u_plus[0] * u_plus[0] + u_plus[1] * u_plus[1] + u_plus[2] * u_plus[2];
    const double gamma_new = std::sqrt(1.0 + u_new2 / (constants::C * constants::C));

    // v_new = u_new / gamma_new
    p.v[0] = u_plus[0] / gamma_new;
    p.v[1] = u_plus[1] / gamma_new;
    p.v[2] = u_plus[2] / gamma_new;

    // x_new = x + v_new * dt
    for (int i = 0; i < 3; ++i)
    {
        p.x[i] += p.v[i] * dt;
    }
}
