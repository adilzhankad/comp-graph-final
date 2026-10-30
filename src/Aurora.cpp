#include "Aurora.h"

#include <cmath>
#include <cstdint>

namespace mla {

Aurora::Aurora() {
    constexpr int segments = 140;
    for (int ribbon = 0; ribbon < 3; ++ribbon) {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve((segments + 1) * 2);
        indices.reserve(segments * 6);
        for (int i = 0; i <= segments; ++i) {
            const float u = static_cast<float>(i) / segments;
            const float phase = u * 8.0f + ribbon * 1.7f;
            const float x = (u - 0.5f) * 112.0f;
            const float z = -60.0f - ribbon * 12.0f + std::sin(phase) * 8.0f;
            const float y = 33.0f + ribbon * 3.5f + std::sin(phase * 1.3f) * 2.5f;
            vertices.push_back({{x, y, z}, {0, 0, 1}, {u, 0}});
            vertices.push_back({{x, y + 12.0f + ribbon * 2.0f, z}, {0, 0, 1}, {u, 1}});
            if (i < segments) {
                const auto a = static_cast<std::uint32_t>(i * 2);
                indices.insert(indices.end(), {a, a + 2, a + 1, a + 1, a + 2, a + 3});
            }
        }
        ribbons_.emplace_back(vertices, indices);
    }
}

void Aurora::draw() const {
    for (const auto& ribbon : ribbons_) ribbon.draw();
}

} // namespace mla
