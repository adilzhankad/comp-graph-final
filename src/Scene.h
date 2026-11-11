#pragma once

#include "Camera.h"
#include "Lake.h"
#include "Lighthouse.h"
#include "Shader.h"
#include "Terrain.h"
#include <filesystem>

namespace mla {

struct SceneSettings {
    float lightAngle = -0.7f;
    // Animation clock, separate from frame delta time so it can be paused.
    float time = 0.0f;
    bool wireframe = false;

    // Lighthouse beacon. `beaconTurns` is the phase of the sweep in whole
    // revolutions and is integrated by the caller, so changing the speed moves
    // the beam on from where it stands instead of jumping it.
    bool beacon = true;
    float beaconRpm = 3.5f;
    float beaconTurns = 0.0f;
};

// Terrain, lake and lighthouse. The remaining prototypes stay separate.
class Scene {
public:
    explicit Scene(const std::filesystem::path& resourceRoot);
    void render(const Camera& camera, float aspect, const SceneSettings& settings);

    // Shared by the clear colour and by the fog every pass fades into.
    static glm::vec3 background() { return {0.18f, 0.25f, 0.29f}; }
    const Lake& lake() const { return lake_; }
    const Lighthouse& lighthouse() const { return lighthouse_; }

private:
    // What every pass needs, assembled once per frame. One object gets one
    // private pass method, so a new object is a new method and one call.
    struct Frame {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::vec3 camera{0.0f};
        glm::vec3 sun{0.0f, 1.0f, 0.0f};
        Beacon beacon;
        bool beaconOn = false;
        float halfSize = 0.0f;
        float time = 0.0f;
    };

    // Call after Shader::use(): the view, the lights and the fog, which every
    // shader declares the same way.
    void applyFrame(const Shader& shader, const Frame& frame) const;
    void renderTerrain(const Frame& frame);
    void renderLighthouse(const Frame& frame);
    void renderLake(const Frame& frame);
    void renderBeam(const Frame& frame);

    Terrain terrain_;
    Lake lake_;
    Lighthouse lighthouse_;
    Shader terrainShader_;
    Shader lakeShader_;
    Shader objectShader_;
    Shader beamShader_;
};

} // namespace mla
