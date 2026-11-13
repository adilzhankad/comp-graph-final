#include "Aurora.h"
#include "Bezier.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace mla {

Aurora::Aurora() {
    // Each lower edge is three cubic Bezier segments: points 0, 3, 6 and 9
    // are shared endpoints; the other points are genuine curve handles.
    // Handles on either side of a join are mirrored, giving continuous
    // tangents as the curtains sweep through their broad, asymmetric folds.
    struct Curtain {
        std::array<glm::vec3, 10> controls;
        float height, phase;
    };
    const std::array<Curtain, 3> curtains{{
        {{{{-250.0f, 104.0f, -220.0f}, {-205.0f, 138.0f, -292.0f},
          {-132.0f, 84.0f, -304.0f}, {-84.0f, 78.0f, -260.0f},
          {-36.0f, 72.0f, -216.0f}, {26.0f, 54.0f, -176.0f},
          {84.0f, 78.0f, -214.0f}, {142.0f, 102.0f, -252.0f},
          {206.0f, 126.0f, -318.0f}, {258.0f, 106.0f, -278.0f}}},
         138.0f, 0.2f},
        {{{{-232.0f, 142.0f, -320.0f}, {-192.0f, 161.0f, -358.0f},
          {-150.0f, 144.0f, -350.0f}, {-104.0f, 128.0f, -326.0f},
          {-58.0f, 112.0f, -302.0f}, {-12.0f, 118.0f, -276.0f},
          {36.0f, 132.0f, -294.0f}, {84.0f, 146.0f, -312.0f},
          {132.0f, 162.0f, -354.0f}, {180.0f, 140.0f, -330.0f}}},
         112.0f, 2.1f},
        {{{{-36.0f, 110.0f, -252.0f}, {-10.0f, 119.0f, -236.0f},
          {18.0f, 100.0f, -202.0f}, {50.0f, 91.0f, -210.0f},
          {82.0f, 82.0f, -218.0f}, {110.0f, 92.0f, -259.0f},
          {140.0f, 105.0f, -265.0f}, {170.0f, 118.0f, -271.0f},
          {208.0f, 136.0f, -242.0f}, {238.0f, 124.0f, -224.0f}}},
         116.0f, 4.6f}
    }};
    // Longer paths need more columns to retain smooth curves and fine rays.
    // Three equal segment intervals put mesh columns exactly on every join.
    constexpr int segmentCount = 3;
    constexpr int columns = 480;
    constexpr int rows = 24;
    for (std::size_t ribbon = 0; ribbon < curtains.size(); ++ribbon) {
        const auto& curtain = curtains[ribbon];
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve((columns + 1) * (rows + 1));
        indices.reserve(columns * rows * 6);
        for (int column = 0; column <= columns; ++column) {
            const float u = static_cast<float>(column) / columns;
            const float pathParameter = u * segmentCount;
            const int segment = std::min(static_cast<int>(pathParameter), segmentCount - 1);
            const std::size_t first = static_cast<std::size_t>(segment) * 3;
            const CubicBezier3 control{{curtain.controls[first], curtain.controls[first + 1],
                                        curtain.controls[first + 2], curtain.controls[first + 3]}};
            const glm::vec3 baseline = cubicBezier(control, pathParameter - segment);
            // Bezier controls define the silhouette and depth. Small height
            // variations only break up the tips; the shader animates the rays.
            const float height = curtain.height *
                (0.92f + 0.07f * std::sin(u * 13.0f + curtain.phase) +
                 0.05f * std::sin(u * 31.0f + curtain.phase * 1.8f));
            for (int row = 0; row <= rows; ++row) {
                const float v = static_cast<float>(row) / rows;
                // The bright rim is at v=0.12; keep it on the Bezier path.
                // Emissive sheets need no normals; x identifies their palette.
                vertices.push_back({baseline + glm::vec3(0.0f, (v - 0.12f) * height, 0.0f),
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
