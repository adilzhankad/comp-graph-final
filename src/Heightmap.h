#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

namespace mla {

// CPU height field shared by mesh generation, prop placement, and tests.
class Heightmap {
public:
    // The defaults decode the committed asset: see scripts/generate_heightmap.py.
    Heightmap(const std::filesystem::path& path, float worldSize = 560.0f,
              float heightScale = 38.0f, float heightOffset = -10.0f);
    Heightmap(int width, int depth, std::vector<float> heights, float worldSize);

    int width() const { return width_; }
    int depth() const { return depth_; }
    float worldSize() const { return worldSize_; }
    float sample(int x, int z) const;
    float heightAt(float worldX, float worldZ) const;
    glm::vec3 normalAt(int x, int z) const;

private:
    int width_ = 0;
    int depth_ = 0;
    float worldSize_ = 0.0f;
    std::vector<float> heights_;
};

} // namespace mla
