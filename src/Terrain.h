#pragma once

#include "Heightmap.h"
#include "Mesh.h"

namespace mla {

class Terrain {
public:
    explicit Terrain(const std::filesystem::path& heightmap);
    void draw() const { mesh_.draw(); }
    float heightAt(float x, float z) const { return heights_.heightAt(x, z); }
    const Heightmap& heights() const { return heights_; }

private:
    static Mesh makeMesh(const Heightmap& heights);
    Heightmap heights_;
    Mesh mesh_;
};

} // namespace mla
