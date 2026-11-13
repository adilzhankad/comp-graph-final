#include "Scene.h"
#include "ForestGen.h"
#include "World.h"

#include <glad/glad.h>
#include <algorithm>
#include <cmath>

namespace mla {
namespace {

glm::vec3 moonDirection(float angle) {
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

// The caller may render into its own framebuffer/viewport. Restore both even
// if shader setup throws, and do not leave clipping enabled for the main pass.
struct ReflectionPassState {
    GLint draw = 0, read = 0, viewport[4] = {};
    GLfloat clearColor[4] = {};
    GLboolean clip = GL_FALSE;
    ReflectionPassState() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        clip = glIsEnabled(GL_CLIP_DISTANCE0);
    }
    ~ReflectionPassState() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        if (clip) glEnable(GL_CLIP_DISTANCE0); else glDisable(GL_CLIP_DISTANCE0);
    }
};

} // namespace

Scene::Scene(const std::filesystem::path& root)
    : terrain_(root / "assets/heightmaps/mountain_lake.pgm"),
      lake_(terrain_.heights()),
      lighthouse_(terrain_.heights(), lighthouseAnchor, lighthouseYaw),
      plane_(),
      skybox_(root / "assets/skybox/stars"),
      forest_(generateForest(terrain_.heights())),
      terrainShader_(root / "shaders/terrain.vert", root / "shaders/terrain.frag"),
      lakeShader_(root / "shaders/lake.vert", root / "shaders/lake.frag"),
      skyboxShader_(root / "shaders/skybox.vert", root / "shaders/skybox.frag"),
      auroraShader_(root / "shaders/aurora.vert", root / "shaders/aurora.frag"),
      objectShader_(root / "shaders/object.vert", root / "shaders/object.frag"),
      treeShader_(root / "shaders/tree.vert", root / "shaders/tree.frag"),
      beamShader_(root / "shaders/beam.vert", root / "shaders/beam.frag") {}

void Scene::applyBeacon(const Shader& shader, const Beacon& beacon) const {
    shader.set("uBeaconPosition", beacon.position);
    shader.set("uBeaconDirection", beacon.direction);
    shader.set("uBeaconColor", beacon.color);
    shader.set("uBeaconCone", beacon.cone);
}

void Scene::renderEnvironment(const glm::mat4& view, const glm::mat4& projection,
                              glm::vec3 eye, glm::vec3 light, const Beacon& beacon,
                              bool wireframe, bool clipWater, const SceneSettings& settings) {
    const float halfSize = terrain_.heights().worldSize() * .5f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    if (clipWater) glEnable(GL_CLIP_DISTANCE0); else glDisable(GL_CLIP_DISTANCE0);

    terrainShader_.use();
    terrainShader_.set("uModel", glm::mat4(1.0f));
    terrainShader_.set("uView", view);
    terrainShader_.set("uProjection", projection);
    terrainShader_.set("uCameraPosition", eye);
    terrainShader_.set("uClipHeight", world::waterLevel);
    terrainShader_.set("uTerrainHalfSize", halfSize);
    terrainShader_.set("uFogColor", background());
    terrainShader_.set("uLightDirection", light);
    applyBeacon(terrainShader_, beacon);
    terrain_.draw();
    // The tower stands clear of the water, so it needs no clip plane of its
    // own: dropping the terrain's here keeps object.vert free of one.
    glDisable(GL_CLIP_DISTANCE0);

    // One draw per section: each carries its own material, and its model matrix
    // is already the product of its place in the tower and the node it hangs on.
    objectShader_.use();
    objectShader_.set("uView", view);
    objectShader_.set("uProjection", projection);
    objectShader_.set("uCameraPosition", eye);
    objectShader_.set("uTerrainHalfSize", halfSize);
    objectShader_.set("uFogColor", background());
    objectShader_.set("uLightDirection", light);
    applyBeacon(objectShader_, beacon);
    for (const auto& part : lighthouse_.parts()) {
        objectShader_.set("uModel", part.model);
        objectShader_.set("uColor", part.color);
        objectShader_.set("uEmission", part.emission);
        part.mesh->draw();
    }

    for (const auto& part : plane_.parts()) {
        objectShader_.set("uModel", part.model);
        objectShader_.set("uColor", part.color);
        objectShader_.set("uEmission", part.emission);
        part.mesh->draw();
    }

    // --- Instanced pine trees ---
    // Drawn after the lighthouse so the GL state (depth test, cull face, polygon
    // mode) is already configured for opaque solid geometry. The tree shader
    // shares the same beacon uniforms as the other shaders; applyBeacon() sets
    // them once and they are valid for both trunk and canopy passes.
    treeShader_.use();
    treeShader_.set("uView", view);
    treeShader_.set("uProjection", projection);
    treeShader_.set("uCameraPosition", eye);
    treeShader_.set("uTerrainHalfSize", halfSize);
    treeShader_.set("uFogColor", background());
    treeShader_.set("uLightDirection", light);
    applyBeacon(treeShader_, beacon);
    treeShader_.set("uIsTrunk", 1);
    forest_.drawTrunk();
    treeShader_.set("uIsTrunk", 0);
    forest_.drawCanopy();

    // Distant sky fills only untouched depth. Ignore camera translation and
    // keep the background filled even when the terrain is in wireframe mode.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    skyboxShader_.use();
    skyboxShader_.set("uView", view);
    skyboxShader_.set("uProjection", projection);
    skyboxShader_.set("uSkybox", 0);
    skyboxShader_.set("uFogColor", background());
    skybox_.bind(0);
    skybox_.draw();
    glDepthFunc(GL_LESS);

    // Light adds through overlapping curtains without sorting or writing depth.
    // Terrain depth still occludes it, including in the mirrored camera pass.
    if (settings.aurora && settings.auroraIntensity > 0.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        auroraShader_.use();
        auroraShader_.set("uModel", glm::mat4(1.0f));
        auroraShader_.set("uView", view);
        auroraShader_.set("uProjection", projection);
        auroraShader_.set("uCameraPosition", eye);
        auroraShader_.set("uTime", settings.auroraTime);
        auroraShader_.set("uSpeed", 1.0f);
        auroraShader_.set("uIntensity", settings.auroraIntensity);
        auroraShader_.set("uPalette", static_cast<int>(settings.auroraPalette));
        aurora_.draw();
        glDisable(GL_BLEND);
    }
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

void Scene::render(const Camera& camera, float aspect, const SceneSettings& settings) {
    const auto view = camera.view();
    const auto projection = camera.projection(aspect);
    const auto light = moonDirection(settings.lightAngle);
    const float halfSize = terrain_.heights().worldSize() * .5f;
    // The hierarchy is updated before anything reads a world matrix: the beam
    // mesh, the lamp and the light the shaders get all come out of the same one.
    lighthouse_.update(settings.beaconTurns);
    plane_.update(settings.time);
    const auto beacon = lighthouse_.beacon(settings.beacon);

    // Reflect world coordinates about y = waterLevel. Using V*M includes the
    // orientation reflection; projective water UVs need no additional Y flip.
    glm::mat4 mirror(1.0f);
    mirror[1][1] = -1.0f;
    mirror[3][1] = 2.0f * world::waterLevel;
    const auto reflectionView = view * mirror;
    const auto reflectionVP = projection * reflectionView;
    GLint viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const bool reflectScene = settings.reflections && camera.position.y > world::waterLevel + .02f &&
                              viewport[2] > 0 && viewport[3] > 0;
    if (reflectScene) {
        // Preserve aspect at high-DPI resolutions, capping the extra pass at
        // 1280x720. Small windows use their full drawable resolution.
        const float scale = std::min({1.0f, 1280.0f / viewport[2], 720.0f / viewport[3]});
        reflection_.resize(std::max(1, static_cast<int>(std::lround(viewport[2] * scale))),
                           std::max(1, static_cast<int>(std::lround(viewport[3] * scale))));
        const ReflectionPassState savedState;
        reflection_.bind();
        glViewport(0, 0, reflection_.width(), reflection_.height());
        glDepthMask(GL_TRUE);
        const auto fog = background();
        glClearColor(fog.r, fog.g, fog.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto reflectedEye = camera.position;
        reflectedEye.y = 2.0f * world::waterLevel - camera.position.y;
        renderEnvironment(reflectionView, projection, reflectedEye, light, beacon, false, true, settings);
    }

    renderEnvironment(view, projection, camera.position, light, beacon, settings.wireframe, false, settings);

    // Transparent pass: the water reads the depth buffer so the ground clips
    // the shoreline, but writes none of its own, so nothing self-occludes.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    lakeShader_.use();
    lakeShader_.set("uModel", glm::mat4(1.0f));
    lakeShader_.set("uView", view);
    lakeShader_.set("uProjection", projection);
    lakeShader_.set("uCameraPosition", camera.position);
    lakeShader_.set("uTerrainHalfSize", halfSize);
    lakeShader_.set("uFogColor", background());
    lakeShader_.set("uLightDirection", light);
    lakeShader_.set("uTime", settings.time);
    lakeShader_.set("uSkybox", 0);
    lakeShader_.set("uReflection", 1);
    lakeShader_.set("uHasReflection", reflectScene ? 1 : 0);
    lakeShader_.set("uReflectionVP", reflectionVP);
    applyBeacon(lakeShader_, beacon);
    if (reflectScene) reflection_.bindColor(1);
    lake_.draw();
    glActiveTexture(GL_TEXTURE0);

    // Additive pass, last: lit air only ever adds light. It still reads depth,
    // so the mountains cut the beam off where it reaches them. The mirrored
    // pass leaves it out: a reflected shaft would have to be sorted against the
    // water it crosses, and the pool of light on the surface is already there.
    if (settings.beacon) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        beamShader_.use();
        beamShader_.set("uModel", lighthouse_.beamModel());
        beamShader_.set("uView", view);
        beamShader_.set("uProjection", projection);
        beamShader_.set("uCameraPosition", camera.position);
        beamShader_.set("uTerrainHalfSize", halfSize);
        beamShader_.set("uFogColor", background());
        beamShader_.set("uBeamColor", beamColor);
        beamShader_.set("uWaterLevel", world::waterLevel);
        lighthouse_.beamMesh().draw();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace mla
