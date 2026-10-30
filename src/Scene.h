#pragma once

#include "Camera.h"
#include "Shader.h"
#include "Terrain.h"
#include <filesystem>

namespace mla {

struct SceneSettings {
    float lightAngle = -0.7f;
    bool wireframe = false;
};

// First art pass: terrain only. Other scene prototypes stay separate for later.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

private:
    Terrain terrain_;
    Shader terrainShader_;
};

} // namespace mla
