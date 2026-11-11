#include "Skybox.h"

#include <stb_image.h>

#include <array>
#include <memory>
#include <stdexcept>
#include <string>

namespace mla {
namespace {

constexpr std::array<const char*, 6> faceNames = {
    "right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png"
};

// The cube only supplies sampling directions. Its apparent distance is set to
// the far depth by the vertex shader, independently of the camera far plane.
constexpr float cubeVertices[] = {
    -1, -1, -1,   1, -1, -1,   1,  1, -1,
    -1, -1, -1,   1,  1, -1,  -1,  1, -1,
     1, -1,  1,  -1, -1,  1,  -1,  1,  1,
     1, -1,  1,  -1,  1,  1,   1,  1,  1,
    -1, -1,  1,  -1, -1, -1,  -1,  1, -1,
    -1, -1,  1,  -1,  1, -1,  -1,  1,  1,
     1, -1, -1,   1, -1,  1,   1,  1,  1,
     1, -1, -1,   1,  1,  1,   1,  1, -1,
    -1,  1, -1,   1,  1, -1,   1,  1,  1,
    -1,  1, -1,   1,  1,  1,  -1,  1,  1,
    -1, -1,  1,   1, -1,  1,   1, -1, -1,
    -1, -1,  1,   1, -1, -1,  -1, -1, -1
};

// Construction can fail on any image. Restore the caller's upload/buffer state
// both on success and while unwinding, before partially created objects die.
struct UploadState {
    GLint unpackAlignment = 4;
    GLint texture = 0;
    GLint vao = 0;
    GLint arrayBuffer = 0;

    UploadState() {
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &texture);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
    }
    ~UploadState() {
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
        glBindTexture(GL_TEXTURE_CUBE_MAP, static_cast<GLuint>(texture));
        glBindVertexArray(static_cast<GLuint>(vao));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(arrayBuffer));
    }
};

} // namespace

Skybox::Skybox(const std::filesystem::path& faceDirectory) {
    try {
        const UploadState savedState;
        GLint maximumSize = 0;
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maximumSize);
        glGenTextures(1, &texture_);
        if (texture_ == 0) throw std::runtime_error("Cannot create skybox cubemap");
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        int faceSize = 0;
        for (std::size_t face = 0; face < faceNames.size(); ++face) {
            const auto path = faceDirectory / faceNames[face];
            int width = 0, height = 0, channels = 0;
            using Image = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
            Image pixels(stbi_load(path.string().c_str(), &width, &height, &channels, 3),
                         stbi_image_free);
            if (!pixels) {
                const char* reason = stbi_failure_reason();
                throw std::runtime_error("Cannot load skybox face: " + path.string() +
                                         (reason ? " (" + std::string(reason) + ")" : ""));
            }
            if (width <= 0 || width != height || (face != 0 && width != faceSize)) {
                throw std::runtime_error("Skybox faces must be square and equally sized: " +
                                         path.string() + " is " + std::to_string(width) + "x" +
                                         std::to_string(height));
            }
            if (width > maximumSize) {
                throw std::runtime_error("Skybox face exceeds this GPU's cubemap size limit: " +
                                         path.string());
            }
            faceSize = width;
            // The current terrain/water pipeline outputs display-space colours.
            // Keep this RGB8 texture in that same space instead of decoding sRGB.
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(face),
                         0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        if (vao_ == 0 || vbo_ == 0) throw std::runtime_error("Cannot create skybox mesh");
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    } catch (...) {
        release();
        throw;
    }
}

Skybox::~Skybox() {
    release();
}

void Skybox::release() noexcept {
    if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
    if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
    if (texture_ != 0) glDeleteTextures(1, &texture_);
    texture_ = vao_ = vbo_ = 0;
}

void Skybox::bind(unsigned unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Skybox::draw() const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

} // namespace mla
