#include "MeteorShower.h"
#include "MeteorMotion.h"

namespace mla {

MeteorShower::MeteorShower()
    : streak_({{{0.0f, -1.0f, 0.0f}, {}, {0.0f, 0.0f}},
               {{1.0f, -1.0f, 0.0f}, {}, {1.0f, 0.0f}},
               {{1.0f, 1.0f, 0.0f}, {}, {1.0f, 1.0f}},
               {{0.0f, 1.0f, 0.0f}, {}, {0.0f, 1.0f}}},
              {0, 1, 2, 0, 2, 3}) {}

void MeteorShower::draw(const Shader& shader, const glm::mat4& view,
                       const glm::mat4& projection, glm::vec3 eye, float time) const {
    shader.use();
    shader.set("uView", view);
    shader.set("uProjection", projection);
    shader.set("uCameraPosition", eye);
    for (std::size_t id = 0; id < meteorCount; ++id) {
        const auto sample = meteorSample(id, time);
        if (sample.brightness <= 0.0f) continue;
        shader.set("uHead", sample.head);
        shader.set("uTail", sample.tail);
        shader.set("uColor", sample.color);
        shader.set("uHalfWidth", sample.halfWidth);
        shader.set("uBrightness", sample.brightness);
        streak_.draw();
    }
}

} // namespace mla
