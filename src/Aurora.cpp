#include "Aurora.h"

#include <array>
#include <cmath>
#include <cstdint>

namespace mla {

Aurora::Aurora() {
    // Real depth preserves folds in the lake's mirrored view; extra rows let
    // high rays lean independently of the comparatively steady lower edge.
    struct Curtain { float width, center, depth, altitude, height, phase; };
    constexpr std::array<Curtain, 3> curtains{{
        {350.0f, -5.0f, -205.0f, 82.0f, 102.0f, 0.2f},
        {210.0f, -67.0f, -258.0f, 112.0f, 79.0f, 2.1f},
        {160.0f, 104.0f, -195.0f, 73.0f, 88.0f, 4.6f}
    }};
    constexpr int columns = 320;
    constexpr int rows = 20;
    for (std::size_t ribbon = 0; ribbon < curtains.size(); ++ribbon) {
        const auto& curtain = curtains[ribbon];
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve((columns + 1) * (rows + 1));
        indices.reserve(columns * rows * 6);
        for (int column = 0; column <= columns; ++column) {
            const float u = static_cast<float>(column) / columns;
            const float phase = curtain.phase;
            const float fold = u * 10.0f + phase;
            const float curlCenter = ribbon == 0 ? 0.61f : 0.38f;
            const float curlU = u - curlCenter;
            const float curlEnvelope = std::exp(-curlU * curlU / 0.026f);
            const float curlPhase = curlU * 17.0f;
            const float horizontalCurl = ribbon == 0 ? 3.0f : 0.0f;
            const float depthCurl = ribbon == 0 ? 10.0f : 0.0f;
            const float x = curtain.center + (u - 0.5f) * curtain.width +
                            curlEnvelope * std::sin(curlPhase) * horizontalCurl;
            const float z = curtain.depth + std::sin(fold) * (ribbon == 0 ? 23.0f : 14.0f) +
                            curlEnvelope * std::cos(curlPhase) * depthCurl;
            // A high sweep on the left dips into one broad curl toward the
            // right. The shorter supporting sheets have different silhouettes.
            const float tilt = ribbon == 0 ? (0.5f - u) * 37.0f :
                               (ribbon == 1 ? (u - 0.5f) * 16.0f : 0.0f);
            const float bottom = curtain.altitude + tilt +
                                 std::sin(u * 7.0f + phase) * 10.0f +
                                 std::sin(u * 15.0f + phase) * 2.0f;
            const float height = curtain.height *
                (0.85f + 0.12f * std::sin(u * 13.0f + phase) +
                 0.08f * std::sin(u * 31.0f + phase * 1.8f));
            for (int row = 0; row <= rows; ++row) {
                const float v = static_cast<float>(row) / rows;
                // Emissive sheets need no normals; x identifies their palette.
                vertices.push_back({{x, bottom + (v - 0.12f) * height, z},
                                    {static_cast<float>(ribbon), 0.0f, 1.0f}, {u, v}});
                if (column < columns && row < rows) {
                    const auto a = static_cast<std::uint32_t>(column * (rows + 1) + row);
                    const auto b = a + rows + 1;
                    indices.insert(indices.end(), {a, b, a + 1, a + 1, b, b + 1});
                }
            }
        }
        ribbons_.emplace_back(vertices, indices);
    }
}

void Aurora::draw() const {
    for (const auto& ribbon : ribbons_) ribbon.draw();
}

} // namespace mla
