#pragma once

#include "Camera.h"
#include "Lake.h"
#include "Shader.h"
#include "Skybox.h"
#include "Terrain.h"
#include <filesystem>

namespace mla {

struct SceneSettings {
    float lightAngle = -0.7f;
    // Animation clock, separate from frame delta time so it can be paused.
    float time = 0.0f;
    bool wireframe = false;
};

// Terrain, starry sky and lake. Aurora remains a separate geometry prototype.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

    // Display-space night haze, shared by sky horizon, terrain and lake fog.
    static glm::vec3 background() { return {0.045f, 0.065f, 0.11f}; }
    const Lake& lake() const { return lake_; }

private:
    Terrain terrain_;
    Lake lake_;
    Skybox skybox_;
    Shader terrainShader_;
    Shader lakeShader_;
    Shader skyboxShader_;
};

} // namespace mla
