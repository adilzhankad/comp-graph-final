#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mla {

glm::vec3 Camera::forward() const {
    const float yaw = glm::radians(yaw_);
    const float pitch = glm::radians(pitch_);
    return glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch),
                                    std::sin(pitch),
                                    std::sin(yaw) * std::cos(pitch)));
}

glm::mat4 Camera::view() const {
    return glm::lookAt(position, position + forward(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projection(float aspect) const {
    if (!std::isfinite(aspect) || aspect <= 0.0f) {
        throw std::invalid_argument("Camera aspect ratio must be finite and positive");
    }
    return glm::perspective(glm::radians(fov_), aspect, 0.1f, 500.0f);
}

void Camera::move(glm::vec3 local, float dt, bool fast) {
    if (!std::isfinite(dt) || dt < 0.0f ||
        !std::isfinite(local.x) || !std::isfinite(local.y) || !std::isfinite(local.z)) {
        throw std::invalid_argument("Camera movement and time step must be finite; time cannot be negative");
    }
    const glm::vec3 front = forward();
    const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 direction = right * local.x + glm::vec3(0.0f, local.y, 0.0f) + front * local.z;
    const float length = glm::length(direction);
    if (length > 1.0f) direction /= length;
    position += direction * dt * (fast ? 24.0f : 8.0f);
}

void Camera::look(float dx, float dy) {
    if (!std::isfinite(dx) || !std::isfinite(dy)) {
        throw std::invalid_argument("Camera look deltas must be finite");
    }
    yaw_ = std::remainder(yaw_ + dx * 0.1f, 360.0f);
    pitch_ = std::clamp(pitch_ + dy * 0.1f, -89.0f, 89.0f);
}

void Camera::zoom(float scroll) {
    if (!std::isfinite(scroll)) throw std::invalid_argument("Camera zoom must be finite");
    fov_ = std::clamp(fov_ - scroll, 15.0f, 75.0f);
}

} // namespace mla
