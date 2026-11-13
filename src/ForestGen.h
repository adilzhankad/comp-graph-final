#pragma once

#include "Heightmap.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <vector>

namespace mla {

// CPU-only forest placement: no OpenGL dependency.
// Scatters tree instance matrices over the terrain according to elevation,
// slope, and cluster-density rules. The generator is fully deterministic: the
// same Heightmap always produces the same result regardless of call order or
// platform.
//
// Placement rules (applied in order, all must pass):
//   1. height > waterLevel + minClearance          — no trees in / on the lake
//   2. height < maxTreelineHeight                  — above this is bare rock/snow
//   3. slope  < maxSlope  (1 - normalY)            — cliffs are tree-free
//   4. density gate derived from elevation band:
//        • foothill band  [minClearance .. foothillTop]  — dense, most trees
//        • slope band     (foothillTop  .. slopeTop]     — thinned by density gate
//        • upper band     (slopeTop     .. maxTreelineHeight) — sparse/isolated
//   5. cluster noise gate: each grid cell has a Wang-hash "cluster weight";
//      cells below clusterThreshold are skipped, creating natural open spaces.
//
// Tree scale:
//   Each accepted point receives a random yaw and a uniform scale drawn from
//   [minScale, maxScale]. These values are 80% of the original design to give
//   appropriately-sized conifers against the mountain background.

struct ForestParams {
    // Grid and jitter
    float gridStep        = 9.0f;    // base spacing between candidate points
    float jitterFraction  = 0.50f;   // fraction of gridStep for position jitter

    // Elevation bands
    float minClearance      = 1.5f;  // metres above waterLevel required (shoreline buffer)
    float foothillTop       = 7.0f;  // upper edge of the dense foothill band
    float slopeTop          = 11.0f; // upper edge of the sparse slope band
    float maxTreelineHeight = 13.0f; // above this no trees grow at all

    // Slope
    float maxSlope          = 0.22f; // 1 - normalY; trees reject steeper ground

    // Cluster gating — the fraction of grid cells that are "open" (treeless gaps)
    // is (1 - clusterThreshold).  0 = every cell can hold a tree, 1 = no trees.
    float clusterThreshold  = 0.38f; // cells below this hash value are skipped

    // Density multipliers per elevation band (probability of surviving the
    // cluster gate is multiplied by these before the threshold test).
    float foothillDensity   = 1.00f; // full density in foothill valleys
    float slopeDensity      = 0.45f; // thinned on mid-slopes
    float upperDensity      = 0.15f; // very sparse near the treeline

    // Per-tree scale variation (80% of the original design)
    float minScale          = 0.60f; // 0.75 * 0.8
    float maxScale          = 1.08f; // 1.35 * 0.8

    float waterLevel        = -1.0f; // must match world::waterLevel
};

// Returns one model matrix per tree. Each matrix positions the tree's base
// (y=0 in local space) on the terrain surface and applies a random yaw and
// uniform scale.
std::vector<glm::mat4> generateForest(const Heightmap& heights,
                                      const ForestParams& params = ForestParams{});

} // namespace mla
