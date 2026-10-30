#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mla {
namespace {

std::string readSource(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open shader: " + path.string());
    std::ostringstream source;
    source << input.rdbuf();
    if (input.bad()) throw std::runtime_error("Cannot read shader: " + path.string());
    auto result = source.str();
    if (result.empty()) throw std::runtime_error("Shader file is empty: " + path.string());
    // A UTF-8 BOM is not valid before GLSL's #version directive.
    if (result.compare(0, 3, "\xEF\xBB\xBF") == 0) result.erase(0, 3);
    return result;
}

std::string shaderLog(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) return "No compiler log was provided.";
    std::vector<char> buffer(static_cast<std::size_t>(length));
    glGetShaderInfoLog(shader, length, nullptr, buffer.data());
    return std::string(buffer.data());
}

std::string programLog(GLuint program) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) return "No linker log was provided.";
    std::vector<char> buffer(static_cast<std::size_t>(length));
    glGetProgramInfoLog(program, length, nullptr, buffer.data());
    return std::string(buffer.data());
}

GLuint compile(GLenum type, const std::filesystem::path& path) {
    const auto source = readSource(path);
    if (source.size() > static_cast<std::size_t>(std::numeric_limits<GLint>::max())) {
        throw std::length_error("Shader source is too large: " + path.string());
    }
    const GLuint shader = glCreateShader(type);
    if (shader == 0) throw std::runtime_error("Cannot create shader: " + path.string());
    try {
        const char* data = source.c_str();
        const auto length = static_cast<GLint>(source.size());
        glShaderSource(shader, 1, &data, &length);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE) {
            throw std::runtime_error("Shader compilation failed (" + path.string() + "):\n" + shaderLog(shader));
        }
    } catch (...) {
        glDeleteShader(shader);
        throw;
    }
    return shader;
}

} // namespace

Shader::Shader(const std::filesystem::path& vertexPath,
               const std::filesystem::path& fragmentPath) {
    GLuint vertex = 0;
    GLuint fragment = 0;
    try {
        vertex = compile(GL_VERTEX_SHADER, vertexPath);
        fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);
        program_ = glCreateProgram();
        if (program_ == 0) throw std::runtime_error("Cannot create shader program");
        glAttachShader(program_, vertex);
        glAttachShader(program_, fragment);
        glLinkProgram(program_);
        GLint linked = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            throw std::runtime_error("Shader link failed (" + vertexPath.string() + ", " +
                                     fragmentPath.string() + "):\n" + programLog(program_));
        }
        glDetachShader(program_, vertex);
        glDetachShader(program_, fragment);
    } catch (...) {
        if (vertex != 0) glDeleteShader(vertex);
        if (fragment != 0) glDeleteShader(fragment);
        if (program_ != 0) glDeleteProgram(program_);
        program_ = 0;
        throw;
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    if (program_ != 0) glDeleteProgram(program_);
}

Shader::Shader(Shader&& other) noexcept
    : program_(std::exchange(other.program_, 0)), locations_(std::move(other.locations_)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (program_ != 0) glDeleteProgram(program_);
        program_ = std::exchange(other.program_, 0);
        locations_ = std::move(other.locations_);
    }
    return *this;
}

void Shader::use() const {
    glUseProgram(program_);
}

GLint Shader::location(const std::string& name) const {
    const auto found = locations_.find(name);
    if (found != locations_.end()) return found->second;
    const GLint result = glGetUniformLocation(program_, name.c_str());
    locations_.emplace(name, result);
    return result;
}

void Shader::set(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(location(name), 1, glm::value_ptr(value));
}

void Shader::set(const std::string& name, float value) const {
    glUniform1f(location(name), value);
}

void Shader::set(const std::string& name, int value) const {
    glUniform1i(location(name), value);
}

} // namespace mla
