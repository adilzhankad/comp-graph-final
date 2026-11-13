#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace mla {

inline constexpr std::size_t meteorCount = 6;

struct MeteorSample {
    glm::vec3 head{0.0f};
    glm::vec3 tail{0.0f};
    glm::vec3 color{0.0f};
    float halfWidth = 0.0f;
    // Zero during the quiet interval between flights, and at either endpoint.
    float brightness = 0.0f;
};

namespace meteorDetails {

inline float randomUnit(std::uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;
    return static_cast<float>(value & 0x00ffffffu) / 16777216.0f;
}

inline float smooth(float low, float high, float value) {
    const float t = std::clamp((value - low) / (high - low), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct Emitter {
    float period;
    float offset;
    glm::vec3 origin;
    glm::vec3 velocity;
};

inline const std::array<Emitter, meteorCount> emitters{{
    {9.7f, 0.33f, {-95.0f, 112.0f, -225.0f}, {64.0f, -39.0f, 5.0f}},
    {12.1f, 3.9f, {106.0f, 130.0f, -248.0f}, {-69.0f, -42.0f, 7.0f}},
    {14.3f, 7.2f, {-130.0f, 126.0f, -238.0f}, {77.0f, -37.0f, -4.0f}},
    {10.9f, 1.8f, {65.0f, 118.0f, -210.0f}, {-59.0f, -34.0f, -6.0f}},
    {15.7f, 10.2f, {-52.0f, 140.0f, -270.0f}, {72.0f, -45.0f, 8.0f}},
    {8.3f, 5.6f, {128.0f, 124.0f, -242.0f}, {-80.0f, -38.0f, 4.0f}}
}};

} // namespace meteorDetails

// Evaluate from the shared scene clock rather than accumulating per-frame
// particles. Pausing is exact, and the mirrored view sees the same flight.
// Invalid input gives an inactive sample without creating non-finite geometry.
inline MeteorSample meteorSample(std::size_t id, float time) {
    if (id >= meteorCount || !std::isfinite(time) || time < 0.0f) return {};
    const auto& emitter = meteorDetails::emitters[id];
    const double elapsed = static_cast<double>(time) + emitter.offset;
    const double cycleNumber = std::floor(elapsed / emitter.period);
    // Keep conversion bounded even for a very large but finite input clock.
    const auto cycle = static_cast<std::uint32_t>(std::fmod(cycleNumber, 65536.0));
    const float age = static_cast<float>(std::fmod(elapsed, emitter.period));
    const auto seed = static_cast<std::uint32_t>(id) * 8273u + cycle * 17389u + 91u;
    auto random = [seed](std::uint32_t channel) {
        return meteorDetails::randomUnit(seed + channel * 1013u);
    };
    const float duration = 1.35f + random(0) * 0.5f;
    if (age <= 0.0f || age >= duration) return {};

    // Each recurrence has a new starting point and trail, but all trajectories
    // descend through a bounded part of the northern sky, above the terrain.
    const glm::vec3 start = emitter.origin + glm::vec3(
        (random(1) - 0.5f) * 27.0f, (random(2) - 0.5f) * 12.0f,
        (random(3) - 0.5f) * 18.0f);
    const glm::vec3 velocity = emitter.velocity * (0.90f + random(4) * 0.18f);
    const float speed = glm::length(velocity);
    const float trailLength = std::min(18.0f + random(5) * 17.0f, speed * age * 0.80f);
    const float progress = age / duration;
    const float envelope = meteorDetails::smooth(0.0f, 0.12f, age) *
        (1.0f - meteorDetails::smooth(0.67f, 1.0f, progress));

    MeteorSample sample;
    sample.head = start + velocity * age;
    sample.tail = sample.head - velocity * (trailLength / speed);
    sample.color = random(6) < 0.70f ? glm::vec3(0.43f, 0.72f, 1.0f)
                                    : glm::vec3(1.0f, 0.67f, 0.36f);
    sample.halfWidth = 0.48f + random(7) * 0.20f;
    sample.brightness = envelope * (0.95f + random(8) * 0.35f);
    return sample;
}

} // namespace mla
