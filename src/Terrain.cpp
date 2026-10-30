#include "Terrain.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace mla {

Terrain::Terrain(const std::filesystem::path& heightmap)
    : heights_(heightmap), mesh_(makeMesh(heights_)) {}

Mesh Terrain::makeMesh(const Heightmap& heights) {
    // Each coarse triangle owns its vertices: normals never blend across edges.
    constexpr int cells = 128;
    constexpr int side = cells + 1;
    const float cellSize = heights.worldSize() / cells;
    std::vector<glm::vec3> points(side * side);
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(cells * cells * 6);
    indices.reserve(cells * cells * 6);
    auto random = [](std::uint32_t value) {
        value ^= value >> 16;
        value *= 0x7feb352du;
        value ^= value >> 15;
        value *= 0x846ca68bu;
        value ^= value >> 16;
        return static_cast<float>(value & 0xffffu) / 65535.0f;
    };
    for (int z = 0; z < side; ++z) {
        for (int x = 0; x < side; ++x) {
            float worldX = (static_cast<float>(x) / cells - .5f) * heights.worldSize();
            float worldZ = (static_cast<float>(z) / cells - .5f) * heights.worldSize();
            // Shared points keep the surface watertight; small offsets break up
            // the square lattice without crossing its edges.
            if (x > 0 && x < cells && z > 0 && z < cells) {
                const auto seed = static_cast<std::uint32_t>(z * side + x);
                worldX += (random(seed * 2 + 17) - .5f) * cellSize * .40f;
                worldZ += (random(seed * 2 + 83) - .5f) * cellSize * .40f;
            }
            points[z * side + x] = {worldX, heights.heightAt(worldX, worldZ), worldZ};
        }
    }
    auto triangle = [&](int a, int b, int c) {
        const glm::vec3 normal = glm::normalize(glm::cross(points[b] - points[a],
                                                         points[c] - points[a]));
        // The provoking vertex's height stays close to the face average, for
        // stable flat material selection in the terrain shader.
        const float average = (points[a].y + points[b].y + points[c].y) / 3.0f;
        if (std::abs(points[a].y - average) < std::abs(points[c].y - average) &&
            std::abs(points[a].y - average) <= std::abs(points[b].y - average)) {
            const int first = a; a = b; b = c; c = first;
        } else if (std::abs(points[b].y - average) < std::abs(points[c].y - average)) {
            const int last = c; c = b; b = a; a = last;
        }
        for (const int index : {a, b, c}) {
            const auto& p = points[index];
            const glm::vec2 uv(p.x / heights.worldSize() + .5f,
                               p.z / heights.worldSize() + .5f);
            indices.push_back(static_cast<std::uint32_t>(vertices.size()));
            vertices.push_back({p, normal, uv * 20.0f});
        }
    };
    for (int z = 0; z < cells; ++z) {
        for (int x = 0; x < cells; ++x) {
            const int a = z * side + x, b = a + side;
            if (random(static_cast<std::uint32_t>(a + 941)) < .5f) {
                triangle(a, b, a + 1);
                triangle(a + 1, b, b + 1);
            } else {
                triangle(a, b, b + 1);
                triangle(a, b + 1, a + 1);
            }
        }
    }
    return Mesh(vertices, indices);
}

} // namespace mla
