#pragma once

#include "ForestGen.h"
#include "Mesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace mla {

// A single instanced draw: one tree mesh (vertices + indices) rendered for
// every matrix supplied at construction.  Mesh is not reused because Mesh
// does not expose its GPU handles; InstancedMesh owns its own VAO/VBOs and
// keeps the base Mesh class simple.
class InstancedMesh {
public:
    // `vertices` and `indices` are the same layout as Mesh (position, normal,
    // UV). `matrices` are the per-instance model transforms.
    InstancedMesh(const std::vector<Vertex>&     vertices,
                  const std::vector<std::uint32_t>& indices,
                  const std::vector<glm::mat4>&  matrices);
    ~InstancedMesh();
    InstancedMesh(const InstancedMesh&)            = delete;
    InstancedMesh& operator=(const InstancedMesh&) = delete;
    InstancedMesh(InstancedMesh&&)                 = delete;
    InstancedMesh& operator=(InstancedMesh&&)      = delete;

    void draw() const;

    int instanceCount() const { return instanceCount_; }

private:
    GLuint  vao_           = 0;
    GLuint  vbo_           = 0;   // vertex geometry
    GLuint  ebo_           = 0;   // element indices
    GLuint  instanceVbo_   = 0;   // per-instance mat4
    GLsizei indexCount_    = 0;
    GLsizei instanceCount_ = 0;
};

// Two-mesh pine tree rendered with hardware instancing.
// The trunk is a narrow hexagonal frustum; the canopy is three stacked cones
// that give the silhouette of a northern pine without expensive geometry.
class Forest {
public:
    // `matrices` come from generateForest(); they are uploaded once to the GPU
    // at construction and never touched again.
    Forest(const std::vector<glm::mat4>& matrices);

    // Calls glDrawElementsInstanced for trunk, then for canopy.
    void draw() const;
    void drawTrunk()  const;
    void drawCanopy() const;

    int treeCount() const;

private:
    static void buildTrunk (std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx);
    static void buildCanopy(std::vector<Vertex>& verts, std::vector<std::uint32_t>& idx);

    InstancedMesh trunk_;
    InstancedMesh canopy_;
};

} // namespace mla
