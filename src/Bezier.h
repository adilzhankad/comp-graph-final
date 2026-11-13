#pragma once

#include <glm/glm.hpp>

#include <array>

namespace mla {

using CubicBezier3 = std::array<glm::vec3, 4>;

// A cubic Bezier segment for t in [0, 1]. De Casteljau interpolation keeps the
// result inside the control hull and includes both endpoints exactly.
inline glm::vec3 cubicBezier(const CubicBezier3& control, float t) {
    const auto a = glm::mix(control[0], control[1], t);
    const auto b = glm::mix(control[1], control[2], t);
    const auto c = glm::mix(control[2], control[3], t);
    return glm::mix(glm::mix(a, b, t), glm::mix(b, c, t), t);
}

} // namespace mla
