#include "Lake.h"

namespace mla {

Lake::Lake()
    : mesh_({{{-35.0f, waterLevel, -37.0f}, {0, 1, 0}, {0, 0}},
             {{ 35.0f, waterLevel, -37.0f}, {0, 1, 0}, {1, 0}},
             {{ 35.0f, waterLevel,  37.0f}, {0, 1, 0}, {1, 1}},
             {{-35.0f, waterLevel,  37.0f}, {0, 1, 0}, {0, 1}}},
            {0, 2, 1, 0, 3, 2}) {}

} // namespace mla
