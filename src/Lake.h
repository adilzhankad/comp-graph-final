#pragma once

#include "Mesh.h"

namespace mla {

// Transparent first milestone surface; reflection framebuffer is a later feature.
class Lake {
public:
    static constexpr float waterLevel = 0.5f;
    Lake();
    void draw() const { mesh_.draw(); }

private:
    Mesh mesh_;
};

} // namespace mla
