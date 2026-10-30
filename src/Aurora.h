#pragma once

#include "Mesh.h"
#include <vector>

namespace mla {

class Aurora {
public:
    Aurora();
    void draw() const;

private:
    std::vector<Mesh> ribbons_;
};

} // namespace mla
