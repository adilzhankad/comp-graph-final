#include "Scene.h"

#include "World.h"

#include <glad/glad.h>
#include <cmath>

namespace mla {
namespace {

glm::vec3 sunDirection(float angle) {
    return glm::normalize(glm::vec3(std::sin(angle), 0.85f, std::cos(angle) * .65f));
}

// The lighthouse stands on the low headland on the east bank, where the ground
// is barely above the water and the shore curves away under the beam. It is
// inside the starting camera's view, to the right of the valley.
const glm::vec2 lighthouseAnchor{29.0f, -5.0f};
// Turns the tower, and with it the phase the sweep starts from.
constexpr float lighthouseYaw = -1.9f;
// The colour of the lit air in the beam, brighter than the lamp's own material.
const glm::vec3 beamColor{1.00f, 0.88f, 0.62f};

} // namespace

Scene::Scene(const std::filesystem::path& root)
    : terrain_(root / "assets/heightmaps/mountain_lake.pgm"),
      lake_(terrain_.heights()),
      lighthouse_(terrain_.heights(), lighthouseAnchor, lighthouseYaw),
      terrainShader_(root / "shaders/terrain.vert", root / "shaders/terrain.frag"),
      lakeShader_(root / "shaders/lake.vert", root / "shaders/lake.frag"),
      objectShader_(root / "shaders/object.vert", root / "shaders/object.frag"),
      beamShader_(root / "shaders/beam.vert", root / "shaders/beam.frag") {}

void Scene::applyFrame(const Shader& shader, const Frame& frame) const {
    shader.set("uView", frame.view);
    shader.set("uProjection", frame.projection);
    shader.set("uCameraPosition", frame.camera);
    shader.set("uFogColor", background());
    shader.set("uTerrainHalfSize", frame.halfSize);
    shader.set("uLightDirection", frame.sun);
    shader.set("uBeaconPosition", frame.beacon.position);
    shader.set("uBeaconDirection", frame.beacon.direction);
    shader.set("uBeaconColor", frame.beacon.color);
    shader.set("uBeaconCone", frame.beacon.cone);
}

void Scene::render(const Camera& camera, float aspect, const SceneSettings& settings) {
    // The hierarchy is updated before anything reads a world matrix: the beam
    // mesh, the lamp and the light the shaders get all come out of the same one.
    lighthouse_.update(settings.beaconTurns);
    Frame frame;
    frame.view = camera.view();
    frame.projection = camera.projection(aspect);
    frame.camera = camera.position;
    frame.sun = sunDirection(settings.lightAngle);
    frame.beacon = lighthouse_.beacon(settings.beacon);
    frame.beaconOn = settings.beacon;
    frame.halfSize = terrain_.heights().worldSize() * .5f;
    frame.time = settings.time;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);

    renderTerrain(frame);
    renderLighthouse(frame);
    renderLake(frame);
    renderBeam(frame);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::renderTerrain(const Frame& frame) {
    terrainShader_.use();
    applyFrame(terrainShader_, frame);
    terrainShader_.set("uModel", glm::mat4(1.0f));
    terrain_.draw();
}

void Scene::renderLighthouse(const Frame& frame) {
    objectShader_.use();
    applyFrame(objectShader_, frame);
    // One draw per section: each carries its own material, and its model matrix
    // is already the product of its place in the tower and the node it hangs on.
    for (const auto& part : lighthouse_.parts()) {
        objectShader_.set("uModel", part.model);
        objectShader_.set("uColor", part.color);
        objectShader_.set("uEmission", part.emission);
        part.mesh->draw();
    }
}

void Scene::renderLake(const Frame& frame) {
    // Transparent pass: the water reads the depth buffer so the ground clips
    // the shoreline, but writes none of its own, so nothing self-occludes.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    lakeShader_.use();
    applyFrame(lakeShader_, frame);
    lakeShader_.set("uModel", glm::mat4(1.0f));
    lakeShader_.set("uTime", frame.time);
    lake_.draw();
}

void Scene::renderBeam(const Frame& frame) {
    if (!frame.beaconOn) return;
    // Additive pass, last: lit air only ever adds light. It still reads depth,
    // so the mountains cut the beam off where it reaches them.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    beamShader_.use();
    applyFrame(beamShader_, frame);
    beamShader_.set("uModel", lighthouse_.beamModel());
    beamShader_.set("uBeamColor", beamColor);
    beamShader_.set("uWaterLevel", world::waterLevel);
    lighthouse_.beamMesh().draw();
}

} // namespace mla
