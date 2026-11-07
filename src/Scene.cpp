#include "Scene.h"

#include <glad/glad.h>
#include <cmath>

namespace mla {
namespace {

glm::vec3 moonDirection(float angle) {
    return glm::normalize(glm::vec3(std::sin(angle), 0.85f, std::cos(angle) * .65f));
}

} // namespace

Scene::Scene(const std::filesystem::path& root)
    : terrain_(root / "assets/heightmaps/mountain_lake.pgm"),
      lake_(terrain_.heights()),
      skybox_(root / "assets/skybox/stars"),
      terrainShader_(root / "shaders/terrain.vert", root / "shaders/terrain.frag"),
      lakeShader_(root / "shaders/lake.vert", root / "shaders/lake.frag"),
      skyboxShader_(root / "shaders/skybox.vert", root / "shaders/skybox.frag") {}

void Scene::render(const Camera& camera, float aspect, const SceneSettings& settings) {
    const auto view = camera.view();
    const auto projection = camera.projection(aspect);
    const auto light = moonDirection(settings.lightAngle);
    const float halfSize = terrain_.heights().worldSize() * .5f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);

    terrainShader_.use();
    terrainShader_.set("uModel", glm::mat4(1.0f));
    terrainShader_.set("uView", view);
    terrainShader_.set("uProjection", projection);
    terrainShader_.set("uCameraPosition", camera.position);
    terrainShader_.set("uTerrainHalfSize", halfSize);
    terrainShader_.set("uFogColor", background());
    terrainShader_.set("uLightDirection", light);
    terrain_.draw();

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
    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);

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
    lake_.draw();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace mla
