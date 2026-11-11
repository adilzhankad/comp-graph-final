#pragma once

#include "Mesh.h"

namespace mla::primitives {
Mesh cube();
// Unit cone: base radius 1 at y=0, apex at y=1.
Mesh cone(unsigned int segments = 24);
// Y-up solid of revolution: `bottomRadius` at y=0, `topRadius` at y=height.
// One flat normal per side face keeps the facets of the low-poly terrain; the
// caps are optional because a stacked section hides the seam under the next one.
Mesh frustum(float bottomRadius, float topRadius, float height,
             unsigned int segments = 16, bool caps = true);
// Hollow cone standing in for a cone of lit air: apex at the origin, opening
// along +z to a mouth of `radius` at z=length. UV.x is the fraction of the way
// from the apex to the mouth, so the shader can fade the shaft out along it.
Mesh beamCone(float radius, float length, unsigned int segments = 32);
}
