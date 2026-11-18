#pragma once

#include "Mesh.h"
#include "SceneNode.h"

#include <vector>

namespace mla {

// A wooden rowing boat on the lake.
class Boat {
public:
    struct Part {
        const Mesh* mesh = nullptr;
        glm::mat4 model{1.0f};
        glm::vec3 color{1.0f};
        float emission = 0.0f;
    };

    Boat();
    Boat(const Boat&) = delete;
    Boat& operator=(const Boat&) = delete;

    // Rows on slowly and rides the swell. Time comes from the scene clock.
    void update(float time);

    const std::vector<Part>& parts() const { return parts_; }

private:
    struct Piece {
        Mesh mesh;
        glm::mat4 offset{1.0f};
        const SceneNode* node = nullptr;
        glm::vec3 color{1.0f};
    };

    void add(Mesh mesh, const glm::mat4& offset, const SceneNode& node, const glm::vec3& color);
    void refresh();

    SceneNode root_;
    std::vector<Piece> pieces_;
    std::vector<Part> parts_;
};

} // namespace mla
