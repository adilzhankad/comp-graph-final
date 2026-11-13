#pragma once

#include "Mesh.h"
#include "Shader.h"

namespace mla {

// One immutable quad is reused for every streak. The scene owns render state.
class MeteorShower {
public:
    MeteorShower();
    void draw(const Shader& shader, const glm::mat4& view,
              const glm::mat4& projection, glm::vec3 eye, float time) const;

private:
    Mesh streak_;
};

} // namespace mla
