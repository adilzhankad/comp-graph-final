#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace mla {

class SceneNode {
public:
    glm::mat4 local{1.0f};
    glm::mat4 world{1.0f};
    std::vector<std::unique_ptr<SceneNode>> children;

    SceneNode& addChild(std::unique_ptr<SceneNode> child);
    void updateWorld(const glm::mat4& parent = glm::mat4(1.0f));
};

} // namespace mla
