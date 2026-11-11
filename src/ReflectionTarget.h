#pragma once

#include <glad/glad.h>

namespace mla {

// Owns a single-sample colour/depth target for planar reflections. Requires a
// current OpenGL context for allocation, use and destruction.
class ReflectionTarget {
public:
    ReflectionTarget() = default;
    ~ReflectionTarget();
    ReflectionTarget(const ReflectionTarget&) = delete;
    ReflectionTarget& operator=(const ReflectionTarget&) = delete;

    // Allocates only when the size changes. Non-positive sizes leave the target
    // unchanged and return false, so minimized windows do not destroy it.
    // Failure preserves the previous target and the caller's GL bindings.
    bool resize(int width, int height);
    // Binds both read and draw framebuffers; the caller owns viewport/state.
    void bind() const;
    // Leaves the requested texture unit active, with the colour texture bound.
    void bindColor(unsigned unit = 0) const;

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

private:
    void release() noexcept;

    GLuint framebuffer_ = 0;
    GLuint color_ = 0;
    GLuint depth_ = 0;
    int width_ = 0;
    int height_ = 0;
};

} // namespace mla
