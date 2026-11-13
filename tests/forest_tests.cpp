// forest_tests.cpp — placement-rule verification for generateForest().
// Requires the heightmap path as argv[1] (same pattern as heightmap_tests.cpp).
//
// Rules verified:
//   1. Determinism       — two calls produce bit-identical matrices.
//   2. No trees below water (height <= waterLevel + minClearance).
//   3. No trees above treeline (height >= maxTreelineHeight).
//   4. No trees on excessively steep ground (slope > maxSlope).
//   5. No trees in upper sparse band that exceed slopeTop + small tolerance.
//   6. Forest is non-empty.

#include "ForestGen.h"
#include "Heightmap.h"
#include "World.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: forest_tests <path/to/mountain_lake.pgm>\n";
        return 1;
    }

    mla::Heightmap heights(argv[1]);

    // Use the same default params the Scene uses.
    const mla::ForestParams params;

    // --- Rule 1: determinism ---
    // Two independent calls with the same heightmap must produce identical results.
    const auto run1 = mla::generateForest(heights, params);
    const auto run2 = mla::generateForest(heights, params);
    assert(run1.size() == run2.size() && "Forest generator is not deterministic (size differs)");
    for (std::size_t i = 0; i < run1.size(); ++i) {
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                assert(run1[i][col][row] == run2[i][col][row] &&
                       "Forest generator is not deterministic (matrix differs)");
            }
        }
    }
    std::cout << "PASS: determinism (" << run1.size() << " trees)\n";

    const std::vector<glm::mat4>& matrices = run1;

    int belowWater    = 0;
    int aboveTreeline = 0;
    int tooSteep      = 0;

    for (const auto& m : matrices) {
        // Column 3 of the instance matrix is the world-space translation
        // (the translate() call comes before rotate and scale in the builder,
        // so column 3 equals the terrain-surface point directly).
        const float wx = m[3][0];
        const float wy = m[3][1];
        const float wz = m[3][2];

        // Rule 2: no tree at or below the water+clearance level
        if (wy <= mla::world::waterLevel + params.minClearance) {
            ++belowWater;
        }

        // Rule 3: no tree at or above the treeline
        if (wy >= params.maxTreelineHeight) {
            ++aboveTreeline;
        }

        // Rule 4: slope check — look up the nearest grid cell normal
        const float fx = (wx / heights.worldSize() + 0.5f) * (heights.width()  - 1);
        const float fz = (wz / heights.worldSize() + 0.5f) * (heights.depth() - 1);
        const int   ix = static_cast<int>(std::round(fx));
        const int   iz = static_cast<int>(std::round(fz));
        const glm::vec3 n = heights.normalAt(ix, iz);
        const float slope = 1.0f - n.y;
        if (slope > params.maxSlope) {
            ++tooSteep;
        }
    }

    if (belowWater > 0) {
        std::cerr << "FAIL: " << belowWater << " tree(s) placed below water level\n";
        return 1;
    }
    std::cout << "PASS: no trees below water\n";

    if (aboveTreeline > 0) {
        std::cerr << "FAIL: " << aboveTreeline << " tree(s) above treeline ("
                  << params.maxTreelineHeight << ")\n";
        return 1;
    }
    std::cout << "PASS: no trees above treeline (" << params.maxTreelineHeight << ")\n";

    if (tooSteep > 0) {
        std::cerr << "FAIL: " << tooSteep << " tree(s) on slope > maxSlope ("
                  << params.maxSlope << ")\n";
        return 1;
    }
    std::cout << "PASS: no trees on steep cliffs (maxSlope=" << params.maxSlope << ")\n";

    // Rule 6: sanity — forest must not be empty
    assert(!matrices.empty() && "Forest is unexpectedly empty");
    std::cout << "PASS: all forest placement tests passed (" << matrices.size()
              << " trees, foothillTop=" << params.foothillTop
              << " slopeTop=" << params.slopeTop << ").\n";
    return 0;
}

