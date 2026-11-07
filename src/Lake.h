#pragma once

#include "Heightmap.h"
#include "Mesh.h"

#include <cstddef>

namespace mla {

// Flat water surface filling the carved basin at world::waterLevel.
//
// An independent, coarser grid samples the same height field: cells far from
// submerged samples are dropped; the remaining ones overlap the shore.
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
