#include "MeteorMotion.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool finite(glm::vec3 value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool equal(const mla::MeteorSample& a, const mla::MeteorSample& b) {
    return a.head == b.head && a.tail == b.tail && a.color == b.color &&
           a.halfWidth == b.halfWidth && a.brightness == b.brightness;
}

void invalidInputs() {
    const mla::MeteorSample empty;
    require(equal(mla::meteorSample(mla::meteorCount, 1.0f), empty), "Invalid emitter was accepted");
    for (const float time : {-1.0f, std::numeric_limits<float>::infinity(),
                            -std::numeric_limits<float>::infinity(),
                            std::numeric_limits<float>::quiet_NaN()}) {
        require(equal(mla::meteorSample(0, time), empty), "Invalid time did not return an inactive sample");
    }
}

void deterministicScheduleAndMotion() {
    constexpr float step = 0.01f;
    constexpr int steps = 8000;
    std::size_t totalActive = 0;
    for (std::size_t id = 0; id < mla::meteorCount; ++id) {
        auto previous = mla::meteorSample(id, 0.0f);
        bool previousActive = previous.brightness > 0.0f;
        std::size_t active = previousActive ? 1u : 0u;
        std::size_t inactive = previousActive ? 0u : 1u;
        std::size_t motionSamples = 0;
        std::vector<glm::vec3> eventStarts;
        if (previousActive) eventStarts.push_back(previous.head);
        for (int sampleIndex = 1; sampleIndex <= steps; ++sampleIndex) {
            const float time = sampleIndex * step;
            const auto sample = mla::meteorSample(id, time);
            require(equal(sample, mla::meteorSample(id, time)), "Same clock produced different meteor state");
            require(finite(sample.head) && finite(sample.tail) && finite(sample.color) &&
                    std::isfinite(sample.halfWidth) && std::isfinite(sample.brightness),
                    "Meteor state contains non-finite values");
            require(sample.brightness >= 0.0f && sample.brightness <= 1.5f,
                    "Meteor brightness escaped its bounded emission range");
            // A lifetime boundary must fade smoothly; a hard reset/teleport
            // should not leave an instant full-brightness streak.
            require(std::abs(sample.brightness - previous.brightness) < 0.4f,
                    "Meteor brightness jumps at a lifetime or cycle boundary");
            const bool activeNow = sample.brightness > 0.0f;
            if (activeNow) {
                ++active;
                require(sample.head.y > 0.0f && sample.tail.y > 0.0f,
                        "A falling star reached or crossed the lake plane");
                require(sample.color.x >= 0.0f && sample.color.y >= 0.0f && sample.color.z >= 0.0f &&
                        glm::length(sample.color) > 0.0f && sample.halfWidth > 0.0f,
                        "Visible meteor has invalid colour or width");
                require(sample.tail.y >= sample.head.y - 0.001f,
                        "Meteor trail is ahead of the descending head");
                if (!previousActive) eventStarts.push_back(sample.head);
                if (previous.brightness > 0.2f && sample.brightness > 0.2f) {
                    const auto movement = sample.head - previous.head;
                    const auto trail = sample.head - sample.tail;
                    require(glm::length(movement) > 0.0001f && glm::length(movement) < 3.0f && movement.y < 0.0f,
                            "Active meteor stopped, teleported or rose instead of falling");
                    require(glm::dot(movement, trail) > 0.0f,
                            "Trail does not point backward along the meteor flight");
                    ++motionSamples;
                }
            } else {
                ++inactive;
            }
            previous = sample;
            previousActive = activeNow;
        }
        require(active > 100 && inactive > active && motionSamples > 20,
                "Emitter lacks a real flight, pause interval or moving trail");
        require(eventStarts.size() >= 3, "Emitter did not produce recurring events");
        bool varied = false;
        for (std::size_t event = 2; event < eventStarts.size(); ++event)
            varied |= glm::length(eventStarts[event] - eventStarts[1]) > 0.1f;
        require(varied, "Repeated events follow exactly the same spawn point");
        totalActive += active;
    }
    require(totalActive > 0, "Meteor shower never becomes visible");

    // Rendering a reflected view or pausing must be able to re-sample any
    // previous instant; samples cannot depend on which view was rendered last.
    const auto before = mla::meteorSample(0, 0.55f);
    require(before.brightness > 0.0f, "Opening scene lacks its introductory meteor");
    for (std::size_t id = 0; id < mla::meteorCount; ++id)
        (void)mla::meteorSample(id, 500.0f + static_cast<float>(id));
    require(equal(before, mla::meteorSample(0, 0.55f)), "Sampling another view/time mutates meteor state");
}
} // namespace

int main() {
    try {
        invalidInputs();
        deterministicScheduleAndMotion();
        std::cout << "Meteor scheduling, fading, motion and determinism tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Meteor test failure: " << error.what() << '\n';
        return 1;
    }
}
