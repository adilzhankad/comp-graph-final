#pragma once

namespace mla::world {

// Height the lake is flooded to. The basin in scripts/generate_heightmap.py is
// carved against this value, and the shoreline is wherever the terrain crosses
// it, so renderer, generator and tests must agree on one number.
inline constexpr float waterLevel = -1.0f;

} // namespace mla::world
