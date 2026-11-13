#include "Forest.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <stdexcept>

namespace mla {
namespace {

constexpr float kTau = 6.28318530718f;

// Append a flat-shaded triangle to the geometry buffers.
void tri(std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx,
         glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    const glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
    const auto base   = static_cast<std::uint32_t>(verts.size());
    verts.push_back({a, n, {0.0f, 0.0f}});
    verts.push_back({b, n, {0.5f, 1.0f}});
    verts.push_back({c, n, {1.0f, 0.0f}});
    idx.insert(idx.end(), {base, base + 1u, base + 2u});
}

// A flat-shaded quad as two triangles.
void quad(std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx,
          glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
    tri(verts, idx, a, b, c);
    tri(verts, idx, a, c, d);
}

} // namespace

// ---------------------------------------------------------------------------
// InstancedMesh
// ---------------------------------------------------------------------------

InstancedMesh::InstancedMesh(const std::vector<Vertex>&        vertices,
                             const std::vector<std::uint32_t>& indices,
                             const std::vector<glm::mat4>&     matrices) {
    if (vertices.empty() || indices.empty() || matrices.empty()) {
        throw std::invalid_argument("InstancedMesh: empty geometry or instance list");
    }
    indexCount_    = static_cast<GLsizei>(indices.size());
    instanceCount_ = static_cast<GLsizei>(matrices.size());

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glGenBuffers(1, &instanceVbo_);

    glBindVertexArray(vao_);

    // --- Geometry VBO (positions 0, 1, 2 — same layout as Mesh) ---
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));
    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));
    // uv (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));

    // --- Element buffer ---
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
                 indices.data(), GL_STATIC_DRAW);

    // --- Per-instance mat4 VBO (locations 3, 4, 5, 6) ---
    // A mat4 occupies four consecutive vec4 attribute slots.
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(matrices.size() * sizeof(glm::mat4)),
                 matrices.data(), GL_STATIC_DRAW);

    for (GLuint col = 0; col < 4; ++col) {
        const GLuint loc = 3u + col;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              reinterpret_cast<void*>(sizeof(glm::vec4) * col));
        glVertexAttribDivisor(loc, 1); // advance once per instance, not per vertex
    }

    glBindVertexArray(0);
}

InstancedMesh::~InstancedMesh() {
    glDeleteBuffers(1, &instanceVbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void InstancedMesh::draw() const {
    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT,
                            nullptr, instanceCount_);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Forest geometry builders
// ---------------------------------------------------------------------------

void Forest::buildTrunk(std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx) {
    // Hexagonal frustum.  All dimensions are 80% of the original design so
    // the trees read as moderately-sized conifers against the mountains.
    // Base at y = 0 (terrain surface).
    constexpr int    segs       = 6;
    constexpr float  botR       = 0.176f;  // 0.22 * 0.8
    constexpr float  topR       = 0.112f;  // 0.14 * 0.8
    constexpr float  h          = 2.08f;   // 2.6  * 0.8

    for (int i = 0; i < segs; ++i) {
        const float a0 = kTau * i       / segs;
        const float a1 = kTau * (i + 1) / segs;
        const glm::vec3 b0(std::cos(a0) * botR, 0.0f, std::sin(a0) * botR);
        const glm::vec3 b1(std::cos(a1) * botR, 0.0f, std::sin(a1) * botR);
        const glm::vec3 t0(std::cos(a0) * topR, h,    std::sin(a0) * topR);
        const glm::vec3 t1(std::cos(a1) * topR, h,    std::sin(a1) * topR);
        quad(verts, idx, b0, t0, t1, b1);
        // bottom cap
        tri(verts, idx, glm::vec3(0.0f), b1, b0);
    }
    // top cap
    for (int i = 0; i < segs; ++i) {
        const float a0 = kTau * i       / segs;
        const float a1 = kTau * (i + 1) / segs;
        const glm::vec3 t0(std::cos(a0) * topR, h, std::sin(a0) * topR);
        const glm::vec3 t1(std::cos(a1) * topR, h, std::sin(a1) * topR);
        tri(verts, idx, glm::vec3(0.0f, h, 0.0f), t0, t1);
    }
}

void Forest::buildCanopy(std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx) {
    // Three stacked cones that read as a layered pine silhouette.
    // Each tier sits a bit higher and is slightly narrower than the one below.
    // Apex at the tier top; base radius flares outward for a characteristic
    // northern-pine droop.
    // All Y offsets and radii scaled to 80% of the original values.

    struct Tier { float baseY; float radius; float height; };
    constexpr Tier tiers[] = {
        {1.20f, 1.92f, 2.24f},   // 1.5*0.8, 2.4*0.8, 2.8*0.8  — lowest, widest
        {2.56f, 1.52f, 2.00f},   // 3.2*0.8, 1.9*0.8, 2.5*0.8  — middle
        {3.84f, 1.04f, 1.68f},   // 4.8*0.8, 1.3*0.8, 2.1*0.8  — top
    };
    constexpr int segs = 7; // heptagonal cones for a craggy look

    for (const auto& t : tiers) {
        const float apex = t.baseY + t.height;
        for (int i = 0; i < segs; ++i) {
            const float a0 = kTau * i       / segs;
            const float a1 = kTau * (i + 1) / segs;
            const glm::vec3 p0(std::cos(a0) * t.radius, t.baseY, std::sin(a0) * t.radius);
            const glm::vec3 p1(std::cos(a1) * t.radius, t.baseY, std::sin(a1) * t.radius);
            const glm::vec3 ap(0.0f, apex, 0.0f);
            // Side face of cone
            tri(verts, idx, p0, ap, p1);
            // Bottom cap disc
            tri(verts, idx, glm::vec3(0.0f, t.baseY, 0.0f), p0, p1);
        }
    }
}

// ---------------------------------------------------------------------------
// Forest
// ---------------------------------------------------------------------------

Forest::Forest(const std::vector<glm::mat4>& matrices) :
    trunk_ ([&] {
        std::vector<Vertex>        verts;
        std::vector<std::uint32_t> idx;
        buildTrunk(verts, idx);
        return InstancedMesh(verts, idx, matrices);
    }()),
    canopy_([&] {
        std::vector<Vertex>        verts;
        std::vector<std::uint32_t> idx;
        buildCanopy(verts, idx);
        return InstancedMesh(verts, idx, matrices);
    }())
{}

void Forest::draw() const {
    trunk_.draw();
    canopy_.draw();
}

void Forest::drawTrunk() const {
    trunk_.draw();
}

void Forest::drawCanopy() const {
    canopy_.draw();
}

int Forest::treeCount() const {
    return trunk_.instanceCount();
}

} // namespace mla
