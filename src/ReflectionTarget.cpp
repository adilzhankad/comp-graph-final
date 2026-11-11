#include "ReflectionTarget.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace mla {
namespace {

struct AllocationState {
    GLint drawFramebuffer = 0;
    GLint readFramebuffer = 0;
    GLint texture = 0;
    GLint renderbuffer = 0;
    GLint unpackBuffer = 0;

    AllocationState() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    }
    ~AllocationState() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer));
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
        glBindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(renderbuffer));
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(unpackBuffer));
    }

    // Resizing a currently bound target keeps that target bound, using its new
    // storage rather than attempting to restore names that are about to die.
    static void replace(GLint& binding, GLuint previous, GLuint replacement) {
        if (previous != 0 && static_cast<GLuint>(binding) == previous) {
            binding = static_cast<GLint>(replacement);
        }
    }
};

struct TargetObjects {
    GLuint framebuffer = 0;
    GLuint color = 0;
    GLuint depth = 0;

    ~TargetObjects() {
        if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
        if (depth) glDeleteRenderbuffers(1, &depth);
        if (color) glDeleteTextures(1, &color);
    }
};

const char* framebufferStatus(GLenum status) {
    switch (status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "incomplete attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "incomplete draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "incomplete read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "unsupported attachment formats";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "incompatible sample counts";
    default: return "unknown framebuffer error";
    }
}

} // namespace

ReflectionTarget::~ReflectionTarget() {
    release();
}

bool ReflectionTarget::resize(int width, int height) {
    if (width <= 0 || height <= 0 || (width == width_ && height == height_)) {
        return false;
    }

    GLint maximumTextureSize = 0;
    GLint maximumRenderbufferSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumTextureSize);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maximumRenderbufferSize);
    const int maximumSize = std::min(maximumTextureSize, maximumRenderbufferSize);
    const std::string size = std::to_string(width) + "x" + std::to_string(height);
    if (width > maximumSize || height > maximumSize) {
        throw std::runtime_error("Reflection target " + size +
                                 " exceeds this GPU's size limit of " +
                                 std::to_string(maximumSize));
    }

    // Temporary storage is deleted before bindings are restored on every exit.
    // The existing target stays intact until the replacement is complete.
    AllocationState savedState;
    TargetObjects next;
    glGenTextures(1, &next.color);
    if (!next.color) throw std::runtime_error("Cannot create reflection colour texture " + size);
    glBindTexture(GL_TEXTURE_2D, next.color);
    // nullptr must mean no image data, rather than an offset into a caller PBO.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glGenRenderbuffers(1, &next.depth);
    if (!next.depth) throw std::runtime_error("Cannot create reflection depth buffer " + size);
    glBindRenderbuffer(GL_RENDERBUFFER, next.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    glGenFramebuffers(1, &next.framebuffer);
    if (!next.framebuffer) throw std::runtime_error("Cannot create reflection framebuffer " + size);
    glBindFramebuffer(GL_FRAMEBUFFER, next.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next.color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, next.depth);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Cannot allocate reflection target " + size + ": " +
                                 framebufferStatus(status) + " (status " +
                                 std::to_string(status) + ")");
    }

    AllocationState::replace(savedState.drawFramebuffer, framebuffer_, next.framebuffer);
    AllocationState::replace(savedState.readFramebuffer, framebuffer_, next.framebuffer);
    AllocationState::replace(savedState.texture, color_, next.color);
    AllocationState::replace(savedState.renderbuffer, depth_, next.depth);
    std::swap(framebuffer_, next.framebuffer);
    std::swap(color_, next.color);
    std::swap(depth_, next.depth);
    width_ = width;
    height_ = height;
    return true;
}

void ReflectionTarget::bind() const {
    if (!framebuffer_) throw std::logic_error("Reflection target must be resized before binding");
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

void ReflectionTarget::bindColor(unsigned unit) const {
    if (!color_) throw std::logic_error("Reflection target must be resized before sampling");
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, color_);
}

void ReflectionTarget::release() noexcept {
    if (framebuffer_) glDeleteFramebuffers(1, &framebuffer_);
    if (depth_) glDeleteRenderbuffers(1, &depth_);
    if (color_) glDeleteTextures(1, &color_);
    framebuffer_ = color_ = depth_ = 0;
    width_ = height_ = 0;
}

} // namespace mla
