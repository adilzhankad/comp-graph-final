#pragma once

#include "Mesh.h"

#include <vector>

namespace mla {

// A small plane in the night sky, built from the same primitives as the
// lighthouse. It is dark except for its lights, so at this distance it reads
// as a silhouette.
class Plane {
public:
    struct Part {
        const Mesh* mesh = nullptr;
        glm::mat4 model{1.0f};
        glm::vec3 color{1.0f};
        float emission = 0.0f;
    };

    Plane();
    Plane(const Plane&) = delete;
    Plane& operator=(const Plane&) = delete;

    const std::vector<Part>& parts() const { return parts_; }

private:
    struct Piece {
        Mesh mesh;
        glm::mat4 offset{1.0f};
        glm::vec3 color{1.0f};
        float emission = 0.0f;
    };

    void add(Mesh mesh, const glm::mat4& offset, const glm::vec3& color, float emission = 0.0f);
    void refresh();

    std::vector<Piece> pieces_;
    std::vector<Part> parts_;
    glm::mat4 model_{1.0f};
};

} // namespace mla
