#include "Texture.h"

#include <stb_image.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace mla {
namespace {

// Decoded pixels, always four 8-bit channels so rows stay 4-byte aligned.
struct Image {
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{nullptr, stbi_image_free};
    int width = 0;
    int height = 0;
};

Image decode(const std::filesystem::path& path, bool flipVertically) {
    // stb_image keeps the flip flag as global state, so set it on every load.
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    Image image;
    int channels = 0;
    image.pixels.reset(stbi_load(path.string().c_str(), &image.width, &image.height,
                                 &channels, STBI_rgb_alpha));
    if (!image.pixels) {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error("Cannot load texture " + path.string() + ": " +
                                 (reason ? reason : "unknown image error"));
    }
    if (image.width <= 0 || image.height <= 0) {
        throw std::runtime_error("Texture has no pixels: " + path.string());
    }
    return image;
}

} // namespace

Texture::Texture(GLenum target) : target_(target) {
    glGenTextures(1, &id_);
    if (id_ == 0) throw std::runtime_error("Cannot create an OpenGL texture");
}

Texture Texture::load2D(const std::filesystem::path& path, ColorSpace colorSpace) {
    const Image image = decode(path, true);
    Texture texture(GL_TEXTURE_2D);
    const GLint internalFormat = colorSpace == ColorSpace::Srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    glBindTexture(GL_TEXTURE_2D, texture.id_);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.get());
    glGenerateMipmap(GL_TEXTURE_2D);
    // Without a mipmapped minification filter the generated levels are never
    // read and distant slopes shimmer.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

Texture Texture::loadCubemap(const std::array<std::filesystem::path, 6>& faces) {
    Texture texture(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.id_);
    int size = 0;
    for (std::size_t i = 0; i < faces.size(); ++i) {
        const Image image = decode(faces[i], false);
        if (image.width != image.height) {
            throw std::runtime_error("Cubemap face is not square: " + faces[i].string());
        }
        if (i == 0) size = image.width;
        if (image.width != size) {
            throw std::runtime_error("Cubemap face size differs from +X face: " +
                                     faces[i].string());
        }
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_RGBA8,
                     image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     image.pixels.get());
    }
    // The sky is always magnified or seen at its native size: no mipmaps.
    // Clamping on every axis keeps filtering from wrapping to the opposite
    // edge of a face, which would show as seams along the cube edges.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return texture;
}

Texture::~Texture() {
    release();
}

Texture::Texture(Texture&& other) noexcept
    : id_(std::exchange(other.id_, 0)),
      target_(std::exchange(other.target_, GL_TEXTURE_2D)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        release();
        id_ = std::exchange(other.id_, 0);
        target_ = std::exchange(other.target_, GL_TEXTURE_2D);
    }
    return *this;
}

void Texture::release() noexcept {
    if (id_ != 0) glDeleteTextures(1, &id_);
    id_ = 0;
}

void Texture::bind(int unit) const {
    if (unit < 0) throw std::out_of_range("Texture unit must not be negative");
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + unit));
    glBindTexture(target_, id_);
}

} // namespace mla
