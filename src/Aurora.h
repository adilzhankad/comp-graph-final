#pragma once

#include "Mesh.h"
#include <vector>

namespace mla {

enum class AuroraPalette { Green, Blue, Orange };

inline const char* auroraPaletteName(AuroraPalette palette) {
    switch (palette) {
    case AuroraPalette::Blue: return "Blue";
    case AuroraPalette::Orange: return "Orange";
    default: return "Green";
    }
}

class Aurora {
public:
    Aurora();
    void draw() const;

private:
    std::vector<Mesh> ribbons_;
};

} // namespace mla
