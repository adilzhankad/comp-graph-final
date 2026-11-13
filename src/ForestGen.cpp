#include "ForestGen.h"

#include <cmath>

namespace mla {
namespace {

// Deterministic integer hash — same function used in Terrain::makeMesh so the
// two subsystems share one RNG family without coupling their source files.
float wang(std::uint32_t v) {
    v ^= v >> 16;
    v *= 0x7feb352du;
    v ^= v >> 15;
    v *= 0x846ca68bu;
    v ^= v >> 16;
    return static_cast<float>(v & 0xffffu) / 65535.0f;
}

} // namespace

std::vector<glm::mat4> generateForest(const Heightmap& heights,
                                      const ForestParams& params) {
    std::vector<glm::mat4> matrices;

    const float half   = heights.worldSize() * 0.5f;
    const float step   = params.gridStep;
    const float jitter = step * params.jitterFraction;

    // Enumerate a regular grid that covers the whole terrain.
    // The grid indices also act as seeds for the per-cell RNG so the result
    // is fully deterministic regardless of iteration order.
    const int cols = static_cast<int>(std::ceil(heights.worldSize() / step));
    const int rows = static_cast<int>(std::ceil(heights.worldSize() / step));

    matrices.reserve(static_cast<std::size_t>(cols * rows) / 5);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const auto seed = static_cast<std::uint32_t>(row * cols + col);

            // --- Cluster / open-space gate ---
            // Each cell gets a "cluster weight" hash.  Cells below the
            // threshold are entirely skipped, leaving natural treeless gaps.
            // The weight is modulated per elevation band after the height
            // lookup, so slopes are automatically sparser than valleys.
            const float cellWeight = wang(seed * 4u + 3u);

            // Base grid position, centred on the terrain.
            const float baseX = -half + (col + 0.5f) * step;
            const float baseZ = -half + (row + 0.5f) * step;

            // Apply per-cell jitter so the forest never looks like a grid.
            const float wx = baseX + (wang(seed * 4u + 7u)  - 0.5f) * jitter * 2.0f;
            const float wz = baseZ + (wang(seed * 4u + 13u) - 0.5f) * jitter * 2.0f;

            // Clamp to terrain bounds (heightAt already clamps, but keep
            // the candidate sensible for the slope query below).
            if (wx <= -half || wx >= half || wz <= -half || wz >= half) continue;

            const float h = heights.heightAt(wx, wz);

            // --- Rule 1: must be above the lake with a clearance buffer ---
            if (h <= params.waterLevel + params.minClearance) continue;

            // --- Rule 2: must be below the treeline ---
            if (h >= params.maxTreelineHeight) continue;

            // --- Rule 3: slope filter ---
            // Convert world position to nearest heightmap cell for the
            // normal query.  heightAt uses the same mapping.
            const float fx = (wx / heights.worldSize() + 0.5f) * (heights.width()  - 1);
            const float fz = (wz / heights.worldSize() + 0.5f) * (heights.depth() - 1);
            const int   ix = static_cast<int>(std::round(fx));
            const int   iz = static_cast<int>(std::round(fz));
            const glm::vec3 n = heights.normalAt(ix, iz);
            const float slope = 1.0f - n.y;
            if (slope > params.maxSlope) continue;

            // --- Rule 4: elevation-band density gate ---
            // Determine which band this point falls in and retrieve the
            // corresponding density multiplier, then apply it to the cell's
            // cluster weight.  Lower multipliers thin the forest on slopes.
            float density;
            if (h <= params.foothillTop) {
                density = params.foothillDensity;
            } else if (h <= params.slopeTop) {
                // Smooth linear falloff across the mid-slope band.
                const float t = (h - params.foothillTop) /
                                (params.slopeTop - params.foothillTop);
                density = params.foothillDensity +
                          t * (params.slopeDensity - params.foothillDensity);
            } else {
                // Upper band: very sparse, isolated trees only.
                const float t = (h - params.slopeTop) /
                                (params.maxTreelineHeight - params.slopeTop);
                density = params.slopeDensity +
                          t * (params.upperDensity - params.slopeDensity);
            }

            // The effective cluster threshold tightens as density drops.
            // A cell passes if its hash weight exceeds the adjusted threshold.
            const float effectiveThreshold = 1.0f - (1.0f - params.clusterThreshold) * density;
            if (cellWeight < effectiveThreshold) continue;

            // --- Per-instance transform ---
            const float yaw   = wang(seed * 4u + 29u) * 6.28318530718f;
            const float scale = params.minScale +
                                wang(seed * 4u + 61u) * (params.maxScale - params.minScale);

            // Place base of tree (y=0 in local space) exactly on the terrain.
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(wx, h, wz));
            m = glm::rotate(m, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::scale(m, glm::vec3(scale));

            matrices.push_back(m);
        }
    }

    return matrices;
}

} // namespace mla
