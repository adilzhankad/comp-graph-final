#include "Skybox.h"

#include <array>
#include <cstdint>
#include <utility>

namespace mla {
namespace {

// Only positions: the interpolated position is the sampling direction, so
// the sky needs no normals or UVs.
constexpr std::array<float, 24> cubeCorners{
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, 1.0f, -1.0f,   -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f, 1.0f,  1.0f,   -1.0f, 1.0f,  1.0f};

// Wound to face inwards, in case face culling is ever enabled.
constexpr std::array<std::uint8_t, 36> cubeFaces{
    0, 1, 2,  0, 2, 3,   // -Z
    4, 6, 5,  4, 7, 6,   // +Z
    0, 7, 4,  0, 3, 7,   // -X
    1, 6, 2,  1, 5, 6,   // +X
    0, 5, 1,  0, 4, 5,   // -Y
    3, 6, 7,  3, 2, 6};  // +Y

std::array<std::filesystem::path, 6> facePaths(const std::filesystem::path& root) {
    const auto folder = root / "assets/skybox";
    // OpenGL face order: +X, -X, +Y, -Y, +Z, -Z.
    return {folder / "px.png", folder / "nx.png", folder / "py.png",
            folder / "ny.png", folder / "pz.png", folder / "nz.png"};
}

} // namespace

Skybox::Skybox(const std::filesystem::path& root)
    : shader_(root / "shaders/skybox.vert", root / "shaders/skybox.frag"),
      cubemap_(Texture::loadCubemap(facePaths(root))) {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeCorners), cubeCorners.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeFaces), cubeFaces.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // The sampler always reads the sky unit; set it once.
    shader_.use();
    shader_.set("uSky", TextureUnit::Sky);
}

Skybox::~Skybox() {
    release();
}

Skybox::Skybox(Skybox&& other) noexcept
    : shader_(std::move(other.shader_)),
      cubemap_(std::move(other.cubemap_)),
      vao_(std::exchange(other.vao_, 0)),
      vbo_(std::exchange(other.vbo_, 0)),
      ebo_(std::exchange(other.ebo_, 0)) {}

Skybox& Skybox::operator=(Skybox&& other) noexcept {
    if (this != &other) {
        release();
        shader_ = std::move(other.shader_);
        cubemap_ = std::move(other.cubemap_);
        vao_ = std::exchange(other.vao_, 0);
        vbo_ = std::exchange(other.vbo_, 0);
        ebo_ = std::exchange(other.ebo_, 0);
    }
    return *this;
}

void Skybox::release() noexcept {
    if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    ebo_ = vbo_ = vao_ = 0;
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection,
                  const glm::vec3& fogColor) const {
    if (vao_ == 0) return;
    // The vertex shader writes depth 1.0, the cleared value; GL_LESS would
    // reject every sky fragment, GL_LEQUAL keeps those nothing covered.
    glDepthFunc(GL_LEQUAL);
    shader_.use();
    // Dropping the translation keeps the cube centred on the camera, so the
    // sky rotates with the view but never comes closer.
    shader_.set("uView", glm::mat4(glm::mat3(view)));
    shader_.set("uProjection", projection);
    shader_.set("uFogColor", fogColor);
    cubemap_.bind(TextureUnit::Sky);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeFaces.size()), GL_UNSIGNED_BYTE,
                   nullptr);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

} // namespace mla
