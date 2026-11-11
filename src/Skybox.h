#pragma once

#include <glad/glad.h>

#include <filesystem>

namespace mla {

// Owns a cubemap and its cube mesh. Requires a current OpenGL context for its
// lifetime. Image rows are loaded without flipping; faces follow OpenGL's
// +X, -X, +Y, -Y, +Z, -Z convention.
class Skybox {
public:
    explicit Skybox(const std::filesystem::path& faceDirectory);
    ~Skybox();
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    // Leaves the requested texture unit active, with the cubemap bound.
    void bind(unsigned unit = 0) const;
    // The caller owns depth, blending, culling and polygon-mode state.
    void draw() const;

private:
    void release() noexcept;

    GLuint texture_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};

} // namespace mla
