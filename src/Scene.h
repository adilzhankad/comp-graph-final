#pragma once

#include "Camera.h"
#include "Lake.h"
#include "Lighthouse.h"
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
    bool wireframe = false;
    bool reflections = true;

    // Lighthouse beacon. `beaconTurns` is the phase of the sweep in whole
    // revolutions and is integrated by the caller, so changing the speed moves
    // the beam on from where it stands instead of jumping it.
    bool beacon = true;
    float beaconRpm = 3.5f;
    float beaconTurns = 0.0f;
};

// Terrain, starry sky, lighthouse and lake. Aurora remains a separate prototype.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

    // Display-space night haze, shared by sky horizon, terrain and lake fog.
    static glm::vec3 background() { return {0.045f, 0.065f, 0.11f}; }
    const Lake& lake() const { return lake_; }
    const Lighthouse& lighthouse() const { return lighthouse_; }

private:
    // Everything solid, drawn for the mirrored pass as well as for the camera,
    // so the tower and the light on the mountains appear in the water too.
    void renderEnvironment(const glm::mat4& view, const glm::mat4& projection,
                           glm::vec3 eye, glm::vec3 light, const Beacon& beacon,
                           bool wireframe, bool clipWater);
    // The lamp as every lit shader receives it. Call after Shader::use().
    void applyBeacon(const Shader& shader, const Beacon& beacon) const;

    Terrain terrain_;
    Lake lake_;
    Lighthouse lighthouse_;
    Skybox skybox_;
    ReflectionTarget reflection_;
    Shader terrainShader_;
    Shader lakeShader_;
    Shader skyboxShader_;
    Shader objectShader_;
    Shader beamShader_;
};

} // namespace mla
