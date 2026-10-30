#include "Scene.h"

#include <glad/glad.h>
#include <cmath>

namespace mla {

Scene::Scene(const std::filesystem::path& root)
    : terrain_(root / "assets/heightmaps/mountain_lake.pgm"),
      terrainShader_(root / "shaders/terrain.vert", root / "shaders/terrain.frag") {}

void Scene::render(const Camera& camera, float aspect, const SceneSettings& settings) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);

    terrainShader_.use();
    terrainShader_.set("uModel", glm::mat4(1.0f));
    terrainShader_.set("uView", camera.view());
    terrainShader_.set("uProjection", camera.projection(aspect));
    terrainShader_.set("uCameraPosition", camera.position);
    terrainShader_.set("uTerrainHalfSize", terrain_.heights().worldSize() * .5f);
    terrainShader_.set("uLightDirection", glm::normalize(glm::vec3(
        std::sin(settings.lightAngle), 0.85f, std::cos(settings.lightAngle) * .65f)));
    terrain_.draw();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace mla
