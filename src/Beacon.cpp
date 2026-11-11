#include "Beacon.h"

#include <cmath>
#include <stdexcept>

namespace mla {

Beacon beaconFromLamp(const glm::mat4& lampWorld, const glm::vec3& color,
                      const glm::vec3& cone) {
    const glm::vec3 axis(lampWorld[2]);
    const float length = glm::length(axis);
    if (!std::isfinite(length) || length < 1e-6f) {
        throw std::invalid_argument("Lamp transform has no usable beam direction");
    }
    const glm::vec3 position(lampWorld[3]);
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
        throw std::invalid_argument("Lamp transform has a non-finite position");
    }
    if (!(cone.x < cone.y) || !(cone.z > 0.0f)) {
        throw std::invalid_argument("Beacon cone needs an outer angle wider than its core and a positive range");
    }
    return Beacon{position, axis / length, color, cone};
}

} // namespace mla
