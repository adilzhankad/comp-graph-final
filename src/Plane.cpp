#include "Plane.h"

#include "Primitives.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <utility>

namespace mla {
namespace {

constexpr float tau = 6.28318530718f;
// A wide circle over the valley, high enough to clear the peaks.
constexpr float radius = 85.0f;
constexpr float altitude = 48.0f;
constexpr float lapSeconds = 52.0f;
constexpr float bank = 0.2f;        // leaned into the turn
// Navigation lights: a short flash, a long gap.
constexpr float blinkEvery = 1.3f;
constexpr float blinkFor = 0.12f;
// Where it starts its lap: in front of the camera rather than behind it.
constexpr float startAngle = tau * .55f;

const glm::vec3 hull{0.17f, 0.18f, 0.21f};
const glm::vec3 lamp{1.00f, 0.12f, 0.08f};

glm::mat4 box(const glm::vec3& centre, const glm::vec3& size) {
    return glm::scale(glm::translate(glm::mat4(1.0f), centre), size);
}

} // namespace

Plane::Plane() {
    pieces_.reserve(7);
    // The frustum stands on its base, so turn it to lie along +z, the way the
    // plane points, and pull it back so the wings sit near the middle.
    const glm::mat4 nose = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) *
                           glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                                       glm::vec3(1.0f, 0.0f, 0.0f));
    add(primitives::frustum(1.15f, 0.5f, 9.0f, 8), nose, hull);
    add(primitives::cube(), box({0.0f, 0.0f, -0.4f}, {15.5f, 0.35f, 2.4f}), hull);
    add(primitives::cube(), box({0.0f, 0.25f, -3.4f}, {5.4f, 0.25f, 1.1f}), hull);
    add(primitives::cube(), box({0.0f, 1.1f, -3.5f}, {0.28f, 2.1f, 1.5f}), hull);
    // Navigation lights: both wingtips and the belly.
    add(primitives::cube(), box({-7.5f, 0.05f, -0.4f}, glm::vec3(0.55f)), lamp, 3.5f);
    add(primitives::cube(), box({7.5f, 0.05f, -0.4f}, glm::vec3(0.55f)), lamp, 3.5f);
    add(primitives::cube(), box({0.0f, -0.8f, 0.6f}, glm::vec3(0.45f)), lamp, 3.5f);

    parts_.resize(pieces_.size());
    update(0.0f);
}

void Plane::update(float time) {
    const float angle = startAngle + time * tau / lapSeconds;
    const glm::vec3 position(std::sin(angle) * radius, altitude, std::cos(angle) * radius);
    // The plane points along +z, so the heading is the tangent of the circle.
    model_ = glm::rotate(glm::translate(glm::mat4(1.0f), position), angle + tau * .25f,
                         glm::vec3(0.0f, 1.0f, 0.0f));
    model_ = glm::rotate(model_, bank, glm::vec3(0.0f, 0.0f, 1.0f));
    blink_ = std::fmod(time, blinkEvery) < blinkFor ? 1.0f : 0.0f;
    refresh();
}

void Plane::add(Mesh mesh, const glm::mat4& offset, const glm::vec3& color, float emission) {
    pieces_.push_back(Piece{std::move(mesh), offset, color, emission});
}

void Plane::refresh() {
    for (std::size_t i = 0; i < pieces_.size(); ++i) {
        const auto& piece = pieces_[i];
        // The mesh address is taken here: the vector may still move its
        // elements while the plane is being built.
        parts_[i] = Part{&piece.mesh, model_ * piece.offset, piece.color,
                         piece.emission * blink_};
    }
}

} // namespace mla
