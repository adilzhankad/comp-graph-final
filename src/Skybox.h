#pragma once

#include "Shader.h"
#include "Texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>

namespace mla {

// Night-sky cubemap drawn on a unit cube around the camera. Owns the cube
// buffers, the skybox shader and the cubemap. Draw it after the opaque
// geometry and before the lake, so it only fills pixels the ground left free.
class Skybox {
public:
    // Loads shaders/skybox.* and assets/skybox/{px,nx,py,ny,pz,nz}.png.
    explicit Skybox(const std::filesystem::path& resourceRoot);
    ~Skybox();
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;
    Skybox(Skybox&& other) noexcept;
    Skybox& operator=(Skybox&& other) noexcept;

    // Leaves the depth function at GL_LESS, as every other pass expects.
    // fogColor is the colour the terrain fades into; the sky meets it at the
    // horizon so the far edge of the ground has no visible seam.
    void draw(const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& fogColor) const;

    // The lake samples the same cubemap for its reflection.
    const Texture& cubemap() const { return cubemap_; }

private:
    void release() noexcept;

    Shader shader_;
    Texture cubemap_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};

} // namespace mla
