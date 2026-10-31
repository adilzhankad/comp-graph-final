#include "Lake.h"

#include "World.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace mla {

// triangleCount_ is declared before mesh_, so it already exists when makeMesh
// runs as part of the member initialiser for mesh_.
Lake::Lake(const Heightmap& heights, int cells) : mesh_(makeMesh(heights, cells)) {}

Mesh Lake::makeMesh(const Heightmap& heights, int cells) {
    if (cells < 1) throw std::invalid_argument("Lake needs at least one surface cell");
    const int side = cells + 1;
    const float size = heights.worldSize();
    auto position = [&](int x, int z) {
        return glm::vec3((static_cast<float>(x) / cells - .5f) * size, world::waterLevel,
                         (static_cast<float>(z) / cells - .5f) * size);
    };

    std::vector<char> submerged(static_cast<std::size_t>(side) * side, 0);
    for (int z = 0; z < side; ++z) {
        for (int x = 0; x < side; ++x) {
            const auto point = position(x, z);
            submerged[static_cast<std::size_t>(z) * side + x] =
                heights.heightAt(point.x, point.z) < world::waterLevel;
        }
    }

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    for (int z = 0; z < cells; ++z) {
        for (int x = 0; x < cells; ++x) {
            // One cell of slack around the submerged samples: a shoreline cell
            // whose corners all sit above water can still dip below it between
            // them, and the extra quads stay hidden behind the ground.
            bool keep = false;
            for (int dz = -1; dz <= 2 && !keep; ++dz) {
                for (int dx = -1; dx <= 2 && !keep; ++dx) {
                    const int sx = x + dx, sz = z + dz;
                    keep = sx >= 0 && sx < side && sz >= 0 && sz < side &&
                           submerged[static_cast<std::size_t>(sz) * side + sx] != 0;
                }
            }
            if (!keep) continue;
            const auto start = static_cast<std::uint32_t>(vertices.size());
            for (const auto corner : {std::pair{x, z}, std::pair{x + 1, z},
                                      std::pair{x + 1, z + 1}, std::pair{x, z + 1}}) {
                const auto point = position(corner.first, corner.second);
                vertices.push_back({point, {0.0f, 1.0f, 0.0f}, glm::vec2(point.x, point.z) * .05f});
            }
            indices.insert(indices.end(), {start, start + 2, start + 1,
                                           start, start + 3, start + 2});
        }
    }
    triangleCount_ = indices.size() / 3;
    return Mesh(vertices, indices);
}

} // namespace mla
