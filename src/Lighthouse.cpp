#include "Lighthouse.h"

#include "Primitives.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mla {
namespace {

constexpr float tau = 6.28318530718f;

// The tower, bottom to top. Sections are expressed as the height their base
// sits at, so moving one does not require recomputing the next.
constexpr float plinthBase = -0.7f;        // sunk into the bank
constexpr float shaftBase = 0.8f;
constexpr float shaftHeight = 13.6f;
constexpr float galleryBase = shaftBase + shaftHeight - 0.5f;
constexpr float lanternFloor = galleryBase + 0.75f;
constexpr float lanternHeight = 3.4f;
constexpr float lampHeight = lanternFloor + lanternHeight * .55f;
constexpr float corniceBase = lanternFloor + lanternHeight;
constexpr unsigned int cagePosts = 6;

// The visible cone of lit air and the spot light share one half-angle, so the
// shaft the camera sees and the patch of ground that brightens are the same.
constexpr float beamLength = 190.0f;
constexpr float beamHalfAngle = 8.5f;      // degrees
constexpr float beamCoreAngle = 2.5f;      // fully lit centre of the cone
// A level beam would pass over the whole lake and only ever reach the far
// mountains. This much nose-down puts its lower half on the water instead,
// close enough to the horizontal to still read as a lighthouse.
constexpr float beamTilt = 14.0f;           // degrees

const glm::vec3 rock{0.20f, 0.23f, 0.23f};
const glm::vec3 whitewash{0.76f, 0.78f, 0.76f};
const glm::vec3 paint{0.46f, 0.16f, 0.14f};
const glm::vec3 slate{0.14f, 0.16f, 0.18f};
const glm::vec3 glass{1.00f, 0.86f, 0.58f};
const glm::vec3 steel{0.33f, 0.32f, 0.30f};
// Still over one, because attenuation takes most of it before the beam reaches
// the far bank - but far below what the same light needed against daylight.
const glm::vec3 beaconColor{1.30f, 1.05f, 0.68f};

glm::mat4 atHeight(float y) {
    return glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, y, 0.0f));
}

glm::mat4 box(const glm::vec3& centre, const glm::vec3& size) {
    return glm::scale(glm::translate(glm::mat4(1.0f), centre), size);
}

} // namespace

Lighthouse::Lighthouse(const Heightmap& heights, glm::vec2 anchor, float yaw)
    : beam_(primitives::beamCone(beamLength * std::tan(glm::radians(beamHalfAngle)),
                                 beamLength, 36)) {
    if (!std::isfinite(anchor.x) || !std::isfinite(anchor.y) || !std::isfinite(yaw)) {
        throw std::invalid_argument("Lighthouse placement must be finite");
    }
    // The plinth reaches below its base, so a foot exactly at the waterline
    // would stand in the lake. Keep the whole bottom section on dry ground.
    const float ground = std::max(heights.heightAt(anchor.x, anchor.y),
                                  world::waterLevel - plinthBase);
    root_.local = glm::rotate(glm::translate(glm::mat4(1.0f),
                                             glm::vec3(anchor.x, ground, anchor.y)),
                              yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    lamp_ = &root_.addChild(std::make_unique<SceneNode>());
    // The lamp turns; the beam leaves it at a fixed angle below the horizontal.
    // Keeping that in its own node means update() only writes one matrix.
    beam_node_ = &lamp_->addChild(std::make_unique<SceneNode>());
    beam_node_->local = glm::rotate(glm::mat4(1.0f), glm::radians(beamTilt),
                                    glm::vec3(1.0f, 0.0f, 0.0f));

    pieces_.reserve(12 + cagePosts);
    add(primitives::frustum(3.7f, 3.2f, 1.6f, 12), atHeight(plinthBase), root_, rock);
    add(primitives::frustum(2.8f, 1.65f, shaftHeight, 16), atHeight(shaftBase), root_, whitewash);
    // Two painted bands, each slightly wider than the shaft it wraps.
    add(primitives::frustum(2.55f, 2.41f, 1.7f, 16), atHeight(4.2f), root_, paint);
    add(primitives::frustum(2.12f, 1.98f, 1.7f, 16), atHeight(9.3f), root_, paint);
    add(primitives::frustum(3.0f, 2.8f, 0.5f, 16), atHeight(galleryBase), root_, slate);
    add(primitives::frustum(2.85f, 2.78f, 0.85f, 16), atHeight(galleryBase + 0.5f), root_, slate);
    add(primitives::frustum(1.85f, 1.85f, 0.25f, 12), atHeight(lanternFloor - 0.25f), root_, slate);
    // An open cage rather than glass: the lamp inside has to stay visible, and
    // no transparent surface has to be sorted against the water.
    for (unsigned int i = 0; i < cagePosts; ++i) {
        const float angle = tau * i / cagePosts;
        add(primitives::cube(),
            box({std::cos(angle) * 1.62f, lanternFloor + lanternHeight * .5f,
                 std::sin(angle) * 1.62f}, {0.26f, lanternHeight, 0.26f}),
            root_, slate);
    }
    add(primitives::frustum(1.95f, 1.78f, 0.35f, 12), atHeight(corniceBase), root_, slate);
    add(primitives::cone(12),
        glm::scale(atHeight(corniceBase + 0.35f), glm::vec3(2.05f, 2.1f, 2.05f)), root_, paint);
    add(primitives::cube(), box({0.0f, corniceBase + 2.6f, 0.0f}, glm::vec3(0.3f)), root_, slate);
    // Under the lamp node: these two turn, and so does the light they stand for.
    add(primitives::frustum(0.55f, 0.5f, 1.3f, 8), atHeight(-0.65f), *lamp_, glass, 1.7f);
    add(primitives::cube(), box({0.0f, 0.0f, -0.8f}, {0.22f, 1.5f, 1.4f}), *lamp_, steel, 0.1f);

    parts_.resize(pieces_.size());
    update(0.0f);
}

void Lighthouse::add(Mesh mesh, const glm::mat4& offset, const SceneNode& node,
                     const glm::vec3& color, float emission) {
    pieces_.push_back(Piece{std::move(mesh), offset, &node, color, emission});
}

void Lighthouse::update(float turns) {
    if (!std::isfinite(turns)) throw std::invalid_argument("Beacon phase must be finite");
    lamp_->local = glm::rotate(atHeight(lampHeight), turns * tau, glm::vec3(0.0f, 1.0f, 0.0f));
    root_.updateWorld();
    for (std::size_t i = 0; i < pieces_.size(); ++i) {
        const auto& piece = pieces_[i];
        // The mesh address is taken here rather than when the piece is added:
        // the vector may still move its elements while the tower is built.
        parts_[i] = Part{&piece.mesh, piece.node->world * piece.offset,
                         piece.color, piece.emission};
    }
}

Beacon Lighthouse::beacon(bool enabled) const {
    return beaconFromLamp(beam_node_->world, enabled ? beaconColor : glm::vec3(0.0f),
                          {std::cos(glm::radians(beamHalfAngle)),
                           std::cos(glm::radians(beamCoreAngle)), beamLength});
}

} // namespace mla
