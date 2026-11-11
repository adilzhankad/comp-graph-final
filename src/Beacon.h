#pragma once

#include <glm/glm.hpp>

namespace mla {

// The lighthouse lamp seen as a light source: a spot light whose axis turns
// with the lamp. The renderer hands these four values to every shader that the
// beam has to light, and shaders/beacon.glsl applies the falloff to them.
struct Beacon {
    glm::vec3 position{0.0f};
    // Unit vector along which the beam leaves the lamp.
    glm::vec3 direction{0.0f, 0.0f, 1.0f};
    // Radiant colour; already zero while the lamp is switched off, so a shader
    // needs no branch and no second uniform for the toggle.
    glm::vec3 color{0.0f};
    // cos(outer half-angle), cos(inner half-angle), world range. Packed because
    // the three shaders receive it as the single uniform uBeaconCone.
    glm::vec3 cone{0.9917f, 0.9994f, 170.0f};
};

// Reads the lamp node's world matrix: its origin is the filament and its local
// +z is the direction the beam leaves it. The lamp hangs under the tower and
// turns about the tower axis, so this one matrix is the whole sweep. Throws if
// the matrix has no usable orientation.
Beacon beaconFromLamp(const glm::mat4& lampWorld, const glm::vec3& color,
                      const glm::vec3& cone);

} // namespace mla
