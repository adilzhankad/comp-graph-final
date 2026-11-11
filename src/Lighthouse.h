#pragma once

#include "Beacon.h"
#include "Heightmap.h"
#include "Mesh.h"
#include "SceneNode.h"

#include <vector>

namespace mla {

// A lighthouse on the lake shore, and the light that turns inside it.
//
// The tower is a stack of low-poly solids of revolution standing on the
// terrain. Under it hangs the lamp node, which turns about the tower axis, and
// under that the beam node, which only tilts the beam down towards the water.
// Those two transforms drive everything the light is made of: the emissive lamp
// mesh, the translucent cone of lit air and the Beacon the terrain and water
// shaders are lit by. The hierarchy is what keeps all of them pointing the
// same way; nothing aims anything a second time.
class Lighthouse {
public:
    // One solid draw: which mesh, where it ended up, and its material.
    struct Part {
        const Mesh* mesh = nullptr;
        glm::mat4 model{1.0f};
        glm::vec3 color{1.0f};
        float emission = 0.0f;
    };

    // `anchor` is the world x/z of the tower axis; the base is dropped onto the
    // terrain there. `yaw` turns the tower, and with it the phase of the sweep.
    Lighthouse(const Heightmap& heights, glm::vec2 anchor, float yaw = 0.0f);
    // Pieces and parts refer to each other and to the nodes by address.
    Lighthouse(const Lighthouse&) = delete;
    Lighthouse& operator=(const Lighthouse&) = delete;

    // `turns` is the beam's phase in whole revolutions. The caller integrates
    // it from the animation clock, so changing the speed moves the beam on from
    // where it stands instead of jumping it.
    void update(float turns);

    // Valid after update(), which every frame calls before anything is drawn.
    const std::vector<Part>& parts() const { return parts_; }
    const Mesh& beamMesh() const { return beam_; }
    // The beam cone is modelled in the beam node's own space, apex at the lamp.
    const glm::mat4& beamModel() const { return beam_node_->world; }
    Beacon beacon(bool enabled) const;

    glm::vec3 foot() const { return glm::vec3(root_.world[3]); }
    glm::vec3 lampPosition() const { return glm::vec3(lamp_->world[3]); }

private:
    struct Piece {
        Mesh mesh;
        glm::mat4 offset{1.0f};    // relative to `node`
        const SceneNode* node = nullptr;
        glm::vec3 color{1.0f};
        float emission = 0.0f;
    };

    void add(Mesh mesh, const glm::mat4& offset, const SceneNode& node,
             const glm::vec3& color, float emission = 0.0f);

    SceneNode root_;
    SceneNode* lamp_ = nullptr;        // turns
    SceneNode* beam_node_ = nullptr;   // turns with the lamp, aimed slightly down
    std::vector<Piece> pieces_;
    std::vector<Part> parts_;      // refreshed by update(), one per piece
    Mesh beam_;
};

} // namespace mla
