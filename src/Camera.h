#pragma once

#include <glm/glm.hpp>

namespace mla {

class Camera {
public:
    glm::vec3 position{0.0f, 9.0f, 36.0f};

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    // Local x = right, y = world up, z = forward; dt is in seconds.
    void move(glm::vec3 local, float dt, bool fast = false);
    // Mouse deltas in pixels. Positive dy looks up.
    void look(float dx, float dy);
    void zoom(float scroll);

private:
    glm::vec3 forward() const;

    float yaw_ = -90.0f;
    float pitch_ = -2.0f;
    float fov_ = 60.0f;
};

} // namespace mla
