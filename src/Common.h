#pragma once

#include <glm/glm.hpp>
#include <cmath>

using namespace glm;

// ================= Constants ================= //
inline constexpr float a0 = 1.0f;
inline constexpr double hbar = 1.0;
inline constexpr double m_e = 1.0;
inline float LightingScaler = 700.0f;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define electron_r 0.25f
#define zmSpeed 10.0

#define CLAMP(x, low, high) \
    (((x) < (low)) ? (low) : (((x) > (high)) ? (high) : (x)))