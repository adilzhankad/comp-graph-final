#include "Boat.h"

#include "Primitives.h"
#include "World.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <utility>
#include <vector>

namespace mla {
namespace {

const glm::vec3 planks{0.33f, 0.22f, 0.14f};
const glm::vec3 trim{0.24f, 0.16f, 0.11f};

// Where the boat sits on the lake, and which way it points.
const glm::vec3 moorings{-15.0f, world::waterLevel - 0.1f, 1.0f};
constexpr float heading = 0.7f;

// Cross-sections of the hull from the transom to the bow. Each one is a
// rectangle: narrow at the keel, wider at the gunwale.
struct Section {
    float z, keelHalf, railHalf, keelY, railY;
};
const Section sections[] = {
    {-2.90f, 0.62f, 0.86f, -0.42f, 0.30f},
    {-1.20f, 0.80f, 1.10f, -0.52f, 0.32f},
    { 0.60f, 0.78f, 1.08f, -0.52f, 0.32f},
    { 2.40f, 0.45f, 0.72f, -0.40f, 0.36f},
    { 3.40f, 0.07f, 0.14f, -0.16f, 0.42f},
};

glm::mat4 box(const glm::vec3& centre, const glm::vec3& size) {
    return glm::scale(glm::translate(glm::mat4(1.0f), centre), size);
}

// A planked hull: one quad per plank between neighbouring sections, with the
// normal taken from the quad itself so the facets stay flat like the terrain.
Mesh hull() {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    auto quad = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
        const glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        const auto start = static_cast<std::uint32_t>(vertices.size());
        vertices.insert(vertices.end(), {{a, normal, {0, 0}}, {b, normal, {1, 0}},
                                         {c, normal, {1, 1}}, {d, normal, {0, 1}}});
        indices.insert(indices.end(), {start, start + 1, start + 2,
                                       start, start + 2, start + 3});
    };

    const int count = static_cast<int>(sizeof(sections) / sizeof(sections[0]));
    for (int i = 0; i + 1 < count; ++i) {
        const Section& a = sections[i];
        const Section& b = sections[i + 1];
        quad({a.keelHalf, a.keelY, a.z}, {b.keelHalf, b.keelY, b.z},
             {b.railHalf, b.railY, b.z}, {a.railHalf, a.railY, a.z});
        quad({-a.railHalf, a.railY, a.z}, {-b.railHalf, b.railY, b.z},
             {-b.keelHalf, b.keelY, b.z}, {-a.keelHalf, a.keelY, a.z});
        quad({-a.keelHalf, a.keelY, a.z}, {-b.keelHalf, b.keelY, b.z},
             {b.keelHalf, b.keelY, b.z}, {a.keelHalf, a.keelY, a.z});
    }
    // Transom, and a floor so the open hull is not see-through from the bank.
    const Section& stern = sections[0];
    quad({-stern.keelHalf, stern.keelY, stern.z}, {stern.keelHalf, stern.keelY, stern.z},
         {stern.railHalf, stern.railY, stern.z}, {-stern.railHalf, stern.railY, stern.z});
    quad({-0.78f, -0.28f, -2.80f}, {0.78f, -0.28f, -2.80f},
         {0.52f, -0.28f, 2.60f}, {-0.52f, -0.28f, 2.60f});
    return Mesh(vertices, indices);
}

} // namespace

Boat::Boat() {
    root_.local = glm::rotate(glm::translate(glm::mat4(1.0f), moorings), heading,
                              glm::vec3(0.0f, 1.0f, 0.0f));

    pieces_.reserve(4);
    add(hull(), glm::mat4(1.0f), root_, planks);
    // Gunwale rail along both sides, and two thwarts to sit on.
    add(primitives::cube(), box({-1.02f, 0.34f, -0.3f}, {0.16f, 0.14f, 5.6f}), root_, trim);
    add(primitives::cube(), box({1.02f, 0.34f, -0.3f}, {0.16f, 0.14f, 5.6f}), root_, trim);
    add(primitives::cube(), box({0.0f, 0.12f, -1.1f}, {1.95f, 0.14f, 0.5f}), root_, trim);

    parts_.resize(pieces_.size());
    root_.updateWorld();
    refresh();
}

void Boat::add(Mesh mesh, const glm::mat4& offset, const SceneNode& node,
               const glm::vec3& color) {
    pieces_.push_back(Piece{std::move(mesh), offset, &node, color});
}

void Boat::refresh() {
    for (std::size_t i = 0; i < pieces_.size(); ++i) {
        const auto& piece = pieces_[i];
        parts_[i] = Part{&piece.mesh, piece.node->world * piece.offset, piece.color, 0.0f};
    }
}

} // namespace mla
