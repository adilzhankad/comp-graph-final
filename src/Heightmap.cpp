#include "Heightmap.h"

#include <stb_image.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mla {

Heightmap::Heightmap(const std::filesystem::path& path, float worldSize,
                     float heightScale, float heightOffset)
    : worldSize_(worldSize) {
    if (!std::isfinite(worldSize) || worldSize <= 0.0f ||
        !std::isfinite(heightScale) || !std::isfinite(heightOffset)) {
        throw std::invalid_argument("Heightmap dimensions and height range must be finite");
    }
    int channels = 0;
    using Image = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    Image pixels(stbi_load(path.string().c_str(), &width_, &depth_, &channels, 1),
                 stbi_image_free);
    if (!pixels) {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error("Cannot load heightmap " + path.string() + ": " +
                                 (reason ? reason : "unknown image error"));
    }
    if (width_ < 2 || depth_ < 2) {
        throw std::runtime_error("Heightmap must be at least 2 by 2 pixels");
    }
    heights_.resize(static_cast<std::size_t>(width_) * depth_);
    for (std::size_t i = 0; i < heights_.size(); ++i) {
        heights_[i] = heightOffset + heightScale * (pixels.get()[i] / 255.0f);
    }
}

Heightmap::Heightmap(int width, int depth, std::vector<float> heights, float worldSize)
    : width_(width), depth_(depth), worldSize_(worldSize), heights_(std::move(heights)) {
    if (width < 2 || depth < 2 || !std::isfinite(worldSize) || worldSize <= 0.0f ||
        heights_.size() != static_cast<std::size_t>(width) * depth) {
        throw std::invalid_argument("Invalid height field dimensions");
    }
    for (float height : heights_) {
        if (!std::isfinite(height)) {
            throw std::invalid_argument("Height field values must be finite");
        }
    }
}

float Heightmap::sample(int x, int z) const {
    x = std::clamp(x, 0, width_ - 1);
    z = std::clamp(z, 0, depth_ - 1);
    return heights_[static_cast<std::size_t>(z) * width_ + x];
}

float Heightmap::heightAt(float worldX, float worldZ) const {
    if (!std::isfinite(worldX) || !std::isfinite(worldZ)) {
        throw std::invalid_argument("Height lookup position must be finite");
    }
    const float x = std::clamp((worldX / worldSize_ + 0.5f) * (width_ - 1),
                               0.0f, static_cast<float>(width_ - 1));
    const float z = std::clamp((worldZ / worldSize_ + 0.5f) * (depth_ - 1),
                               0.0f, static_cast<float>(depth_ - 1));
    const int ix = static_cast<int>(std::floor(x));
    const int iz = static_cast<int>(std::floor(z));
    const float top = glm::mix(sample(ix, iz), sample(ix + 1, iz), x - ix);
    const float bottom = glm::mix(sample(ix, iz + 1), sample(ix + 1, iz + 1), x - ix);
    return glm::mix(top, bottom, z - iz);
}

glm::vec3 Heightmap::normalAt(int x, int z) const {
    x = std::clamp(x, 0, width_ - 1);
    z = std::clamp(z, 0, depth_ - 1);
    const int left = std::max(0, x - 1), right = std::min(width_ - 1, x + 1);
    const int back = std::max(0, z - 1), front = std::min(depth_ - 1, z + 1);
    const float dx = (right - left) * worldSize_ / (width_ - 1);
    const float dz = (front - back) * worldSize_ / (depth_ - 1);
    const float slopeX = (sample(right, z) - sample(left, z)) / dx;
    const float slopeZ = (sample(x, front) - sample(x, back)) / dz;
    return glm::normalize(glm::vec3(-slopeX, 1.0f, -slopeZ));
}

} // namespace mla
