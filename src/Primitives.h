#pragma once

#include "Mesh.h"

namespace mla::primitives {
Mesh cube();
// Unit cone: base radius 1 at y=0, apex at y=1.
Mesh cone(unsigned int segments = 24);
}
