#pragma once

#include <glad/glad.h>

#include <array>
#include <filesystem>

namespace mla {

// Owns one OpenGL texture object. Every image is decoded once, when the
// texture is created at startup; nothing is read from disk while rendering.
// Destroy textures before destroying their OpenGL context.
class Texture {
public:
    // Colour images are stored as sRGB, so shaders sample linear values and
    // light them before the final gamma step. Data images (normal maps, masks)
    // must stay linear.
    enum class ColorSpace { Srgb, Linear };

    // Repeating, mipmapped 2D texture, flipped so that v = 0 is the image bottom.
    static Texture load2D(const std::filesystem::path& path,
                          ColorSpace colorSpace = ColorSpace::Srgb);
    // Faces in OpenGL order: +X, -X, +Y, -Y, +Z, -Z. Not flipped: cubemaps
    // follow their own orientation convention. Stored as plain RGB, so a sample
    // is already a display-space colour.
    static Texture loadCubemap(const std::array<std::filesystem::path, 6>& faces);

    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Makes `unit` active and binds this texture to it. The sampler uniform
    // must be set to the same number, see TextureUnit below.
    void bind(int unit) const;

private:
    explicit Texture(GLenum target);
    void release() noexcept;

    GLuint id_ = 0;
    GLenum target_ = GL_TEXTURE_2D;
};

// Fixed texture-unit assignment shared by every shader. A sampler uniform is
// set to its unit number, and the texture is bound to that unit explicitly
// before each draw; nothing relies on whichever unit happened to be active.
namespace TextureUnit {
constexpr int Grass = 0; // terrain.frag uGrass
constexpr int Rock = 1;  // terrain.frag uRock
constexpr int Snow = 2;  // terrain.frag uSnow
constexpr int Sky = 3;   // skybox.frag and lake.frag uSky (cubemap)
} // namespace TextureUnit

} // namespace mla
