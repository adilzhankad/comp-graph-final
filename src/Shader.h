#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace mla {

// Requires a current OpenGL context throughout its lifetime.
class Shader {
public:
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& fragmentPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    // Call use() before setting uniforms.
    void set(const std::string& name, const glm::mat4& value) const;
    void set(const std::string& name, const glm::vec3& value) const;
    void set(const std::string& name, float value) const;
    void set(const std::string& name, int value) const;

private:
    GLint location(const std::string& name) const;

    GLuint program_ = 0;
    mutable std::unordered_map<std::string, GLint> locations_;
};

} // namespace mla
