#include "Primitives.h"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace mla::primitives {
namespace {

constexpr float tau = 6.28318530718f;

void addTriangle(std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices,
                 glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal,
                 glm::vec2 ua, glm::vec2 ub, glm::vec2 uc) {
    const auto start = static_cast<std::uint32_t>(vertices.size());
    vertices.insert(vertices.end(), {{a, normal, ua}, {b, normal, ub}, {c, normal, uc}});
    indices.insert(indices.end(), {start, start + 1, start + 2});
}

} // namespace

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

Mesh frustum(float bottomRadius, float topRadius, float height,
             unsigned int segments, bool caps) {
    if (segments < 3) throw std::invalid_argument("Frustum needs at least 3 segments");
    if (bottomRadius < 0.0f || topRadius < 0.0f || bottomRadius + topRadius <= 0.0f) {
        throw std::invalid_argument("Frustum radii cannot be negative or both zero");
    }
    if (!(height > 0.0f)) throw std::invalid_argument("Frustum height must be positive");

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    auto ring = [&](float radius, float y, unsigned int i) {
        const float angle = tau * i / segments;
        return glm::vec3(std::cos(angle) * radius, y, std::sin(angle) * radius);
    };
    for (unsigned int i = 0; i < segments; ++i) {
        const glm::vec3 b0 = ring(bottomRadius, 0.0f, i), b1 = ring(bottomRadius, 0.0f, i + 1);
        const glm::vec3 t0 = ring(topRadius, height, i), t1 = ring(topRadius, height, i + 1);
        // The side normal is taken from the slant analytically rather than from
        // the triangle, so a degenerate end (a radius of zero) still shades.
        const float middle = tau * (i + .5f) / segments;
        const glm::vec3 normal = glm::normalize(glm::vec3(std::cos(middle) * height,
                                                          bottomRadius - topRadius,
                                                          std::sin(middle) * height));
        const float u0 = static_cast<float>(i) / segments;
        const float u1 = static_cast<float>(i + 1) / segments;
        if (topRadius > 0.0f) {
            addTriangle(vertices, indices, b0, t0, t1, normal, {u0, 0}, {u0, 1}, {u1, 1});
        }
        if (bottomRadius > 0.0f) {
            addTriangle(vertices, indices, b0, t1, b1, normal, {u0, 0}, {u1, 1}, {u1, 0});
        }
    }
    if (caps) {
        for (unsigned int i = 0; i < segments; ++i) {
            auto planar = [&](glm::vec3 point, float radius) {
                return glm::vec2(.5f + .5f * point.x / radius, .5f + .5f * point.z / radius);
            };
            if (bottomRadius > 0.0f) {
                const glm::vec3 a = ring(bottomRadius, 0.0f, i), b = ring(bottomRadius, 0.0f, i + 1);
                addTriangle(vertices, indices, glm::vec3(0.0f, 0.0f, 0.0f), a, b, {0, -1, 0},
                            {.5f, .5f}, planar(a, bottomRadius), planar(b, bottomRadius));
            }
            if (topRadius > 0.0f) {
                const glm::vec3 a = ring(topRadius, height, i), b = ring(topRadius, height, i + 1);
                addTriangle(vertices, indices, glm::vec3(0.0f, height, 0.0f), b, a, {0, 1, 0},
                            {.5f, .5f}, planar(b, topRadius), planar(a, topRadius));
            }
        }
    }
    return Mesh(vertices, indices);
}

Mesh beamCone(float radius, float length, unsigned int segments) {
    if (segments < 3) throw std::invalid_argument("Beam cone needs at least 3 segments");
    if (!(radius > 0.0f) || !(length > 0.0f)) {
        throw std::invalid_argument("Beam cone radius and length must be positive");
    }
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    for (unsigned int i = 0; i < segments; ++i) {
        const float a = tau * i / segments;
        const float b = tau * (i + 1) / segments;
        const glm::vec3 p(std::cos(a) * radius, std::sin(a) * radius, length);
        const glm::vec3 q(std::cos(b) * radius, std::sin(b) * radius, length);
        // Perpendicular to the slant: the shell is open, so both of its sides
        // are drawn and the shader needs the true grazing angle on each.
        const float middle = (a + b) * .5f;
        const glm::vec3 normal = glm::normalize(glm::vec3(std::cos(middle) * length,
                                                          std::sin(middle) * length, -radius));
        addTriangle(vertices, indices, glm::vec3(0.0f), p, q, normal,
                    {0.0f, .5f}, {1.0f, static_cast<float>(i) / segments},
                    {1.0f, static_cast<float>(i + 1) / segments});
    }
    return Mesh(vertices, indices);
}

} // namespace mla::primitives
