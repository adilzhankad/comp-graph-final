#pragma once

#include "Heightmap.h"
#include "Mesh.h"

#include <cstddef>

namespace mla {

// Flat water surface filling the carved basin at world::waterLevel.
//
// The surface is a subset of the terrain grid: cells far from any submerged
// sample are dropped, and the ones that remain are left to overlap the shore.
// The depth test clips them against the ground, so the shoreline follows the
// terrain exactly instead of being approximated by the water mesh.
class Lake {
public:
    explicit Lake(const Heightmap& heights, int cells = 128);
    void draw() const { mesh_.draw(); }
    std::size_t triangleCount() const { return triangleCount_; }

private:
    Mesh makeMesh(const Heightmap& heights, int cells);

    std::size_t triangleCount_ = 0;
    Mesh mesh_;
};

} // namespace mla
