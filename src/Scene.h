#pragma once

#include "Aurora.h"
#include "Camera.h"
#include "Lake.h"
#include "ReflectionTarget.h"
#include "Shader.h"
#include "Skybox.h"
#include "Terrain.h"
#include <filesystem>

namespace mla {

struct SceneSettings {
    float lightAngle = -0.7f;
    // Animation clock, separate from frame delta time so it can be paused.
    float time = 0.0f;
    float auroraIntensity = 1.0f;
    // Accumulated independently so changing speed never jumps animation phase.
    float auroraTime = 0.0f;
    float auroraSpeed = 1.0f;
    bool aurora = true;
    bool wireframe = false;
    bool reflections = true;
};

// Terrain, starry sky, emissive aurora curtains and their reflection in the lake.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

    // Display-space night haze, shared by sky horizon, terrain and lake fog.
    static glm::vec3 background() { return {0.045f, 0.065f, 0.11f}; }
    const Lake& lake() const { return lake_; }

private:
    void renderEnvironment(const glm::mat4& view, const glm::mat4& projection,
                           glm::vec3 eye, glm::vec3 light, bool wireframe, bool clipWater,
                           const SceneSettings& settings);
    Terrain terrain_;
    Lake lake_;
    Skybox skybox_;
    Aurora aurora_;
    ReflectionTarget reflection_;
    Shader terrainShader_;
    Shader lakeShader_;
    Shader skyboxShader_;
    Shader auroraShader_;
};

} // namespace mla
