#pragma once

#include "Camera.h"
#include "Lake.h"
#include "Shader.h"
#include "Terrain.h"
#include <filesystem>

namespace mla {

struct SceneSettings {
    float lightAngle = -0.7f;
    // Animation clock, separate from frame delta time so it can be paused.
    float time = 0.0f;
    bool wireframe = false;
};

// Terrain and lake. The remaining scene prototypes stay separate for later.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

    // Shared by the clear colour and by the fog both passes fade into.
    static glm::vec3 background() { return {0.18f, 0.25f, 0.29f}; }
    const Lake& lake() const { return lake_; }

private:
    Terrain terrain_;
    Lake lake_;
    Shader terrainShader_;
    Shader lakeShader_;
};

} // namespace mla
