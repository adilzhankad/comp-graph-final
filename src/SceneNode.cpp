#include "SceneNode.h"

#include <stdexcept>
#include <utility>

namespace mla {

SceneNode& SceneNode::addChild(std::unique_ptr<SceneNode> child) {
    if (!child) throw std::invalid_argument("Cannot add a null scene child");
    children.push_back(std::move(child));
    return *children.back();
}

void SceneNode::updateWorld(const glm::mat4& parent) {
    world = parent * local;
    for (const auto& child : children) {
        // The public children container permits empty slots during scene edits.
        if (child) child->updateWorld(world);
    }
}

} // namespace mla
