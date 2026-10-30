#include "Primitives.h"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace mla::primitives {

Mesh cube() {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    auto face = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 normal) {
        const auto start = static_cast<std::uint32_t>(vertices.size());
        vertices.insert(vertices.end(), {{a, normal, {0, 0}}, {b, normal, {1, 0}},
                                         {c, normal, {1, 1}}, {d, normal, {0, 1}}});
        indices.insert(indices.end(), {start, start + 1, start + 2,
                                       start, start + 2, start + 3});
    };
    face({-.5f,-.5f,.5f}, {.5f,-.5f,.5f}, {.5f,.5f,.5f}, {-.5f,.5f,.5f}, {0,0,1});
    face({.5f,-.5f,-.5f}, {-.5f,-.5f,-.5f}, {-.5f,.5f,-.5f}, {.5f,.5f,-.5f}, {0,0,-1});
    face({.5f,-.5f,.5f}, {.5f,-.5f,-.5f}, {.5f,.5f,-.5f}, {.5f,.5f,.5f}, {1,0,0});
    face({-.5f,-.5f,-.5f}, {-.5f,-.5f,.5f}, {-.5f,.5f,.5f}, {-.5f,.5f,-.5f}, {-1,0,0});
    face({-.5f,.5f,.5f}, {.5f,.5f,.5f}, {.5f,.5f,-.5f}, {-.5f,.5f,-.5f}, {0,1,0});
    face({-.5f,-.5f,-.5f}, {.5f,-.5f,-.5f}, {.5f,-.5f,.5f}, {-.5f,-.5f,.5f}, {0,-1,0});
    return Mesh(vertices, indices);
}

Mesh cone(unsigned int segments) {
    if (segments < 3) throw std::invalid_argument("Cone needs at least 3 segments");
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    constexpr float tau = 6.28318530718f;
    for (unsigned int i = 0; i < segments; ++i) {
        const float a = tau * i / segments;
        const float b = tau * (i + 1) / segments;
        const glm::vec3 p(std::cos(a), 0.0f, std::sin(a));
        const glm::vec3 q(std::cos(b), 0.0f, std::sin(b));
        const glm::vec3 normal = glm::normalize(glm::vec3(std::cos((a+b)*.5f), 1,
                                                         std::sin((a+b)*.5f)));
        const auto start = static_cast<std::uint32_t>(vertices.size());
        vertices.insert(vertices.end(), {{p,normal,{0,0}}, {{0,1,0},normal,{.5f,1}},
                                         {q,normal,{1,0}}, {p,{0,-1,0},{0,0}},
                                         {q,{0,-1,0},{1,0}}, {{0,0,0},{0,-1,0},{.5f,.5f}}});
        indices.insert(indices.end(), {start, start+1, start+2, start+3, start+4, start+5});
    }
    return Mesh(vertices, indices);
}

} // namespace mla::primitives
