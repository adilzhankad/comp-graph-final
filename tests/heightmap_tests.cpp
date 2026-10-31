#include "Heightmap.h"
#include "World.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
int failures = 0;
void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
bool near(float a, float b, float tolerance = 0.0001f) {
    return std::abs(a - b) <= tolerance;
}
template<typename Function>
void rejects(Function operation, const std::string& message) {
    try {
        operation();
        check(false, message);
    } catch (const std::invalid_argument&) {
        // Expected validation failure.
    }
}
} // namespace

int main(int argc, char** argv) {
    try {
        // Samples of y=2*x+3*z, in a rectangular 3x2 image spanning [-2,2]^2.
        const mla::Heightmap ramp(3, 2, {-10,-6,-2, 2,6,10}, 4.0f);
        check(near(ramp.heightAt(0, 0), 0), "bilinear centre");
        check(near(ramp.heightAt(-1, 1), 1), "bilinear interpolation on rectangular grid");
        check(near(ramp.heightAt(-2, -2), -10), "negative world corner");
        check(near(ramp.heightAt(2, 2), 10), "positive world corner");
        check(near(ramp.heightAt(-200, -200), -10), "out-of-bounds negative clamp");
        check(near(ramp.heightAt(200, 200), 10), "out-of-bounds positive clamp");
        check(near(ramp.sample(-5, 99), 2), "integer sample clamp");
        const glm::vec3 expected = glm::normalize(glm::vec3(-2, 1, -3));
        for (int z = 0; z < 2; ++z) {
            for (int x = 0; x < 3; ++x) {
                const auto n = ramp.normalAt(x, z);
                check(glm::length(n - expected) < .0001f,
                      "finite differences preserve slope at borders");
            }
        }
        const mla::Heightmap flat(2, 2, {5,5,5,5}, 10.0f);
        check(glm::length(flat.normalAt(0, 0) - glm::vec3(0,1,0)) < .0001f,
              "flat terrain normal points up");
        rejects([] { mla::Heightmap bad(1, 2, {0,0}, 4); }, "reject one-pixel dimension");
        rejects([] { mla::Heightmap bad(2, 2, {0,0}, 4); }, "reject missing height samples");
        rejects([] { mla::Heightmap bad(2, 2, {0,0,0,0}, 0); }, "reject zero world size");
        rejects([] {
            mla::Heightmap bad(2, 2, {0,0,0,std::numeric_limits<float>::infinity()}, 4);
        }, "reject non-finite heights");
        rejects([&] { ramp.heightAt(std::numeric_limits<float>::quiet_NaN(), 0); },
                "reject invalid lookup positions");

        if (argc > 1) {
            const mla::Heightmap scene{std::filesystem::path(argv[1])};
            check(scene.width() == 513 && scene.depth() == 513, "decode committed PGM dimensions");
            check(scene.heightAt(0, 0) < .5f, "terrain retains its central valley");
            float maximum = -1000.0f;
            float minimum = 1000.0f;
            std::size_t submerged = 0;
            for (int z = 0; z < scene.depth(); ++z) {
                for (int x = 0; x < scene.width(); ++x) {
                    maximum = std::max(maximum, scene.sample(x, z));
                    minimum = std::min(minimum, scene.sample(x, z));
                    submerged += scene.sample(x, z) < mla::world::waterLevel;
                    check(near(glm::length(scene.normalAt(x, z)), 1), "terrain normals stay unit length");
                }
            }
            check(maximum > 20, "heightmap contains mountain peaks");
            check(maximum < 27.5f, "mountain summits fit the image range without clipping");
            check(minimum > -9.9f, "lake bed fits the image range without clipping");

            // The lake exists only where the ground is carved below the water
            // level, so the basin and its banks are a heightmap property.
            const auto area = static_cast<float>(scene.width()) * scene.depth();
            check(submerged > area * .005f && submerged < area * .05f,
                  "the basin floods a lake-sized part of the valley, not a pond or a sea");
            check(scene.heightAt(0, 10) < mla::world::waterLevel - 5.0f,
                  "the basin centre is deep enough to read as water, not a puddle");
            for (const auto bank : {glm::vec2(0, 120), glm::vec2(0, -90),
                                    glm::vec2(110, 10), glm::vec2(-110, 10)}) {
                check(scene.heightAt(bank.x, bank.y) > mla::world::waterLevel,
                      "the valley rises above the water level around the basin");
            }
            // The southern bank is open valley, so the profile from the bed
            // out to the plain must rise gently and break the surface once.
            // A stepped rim or a second crossing would show as a cliff or a
            // ring of puddles at the shoreline.
            int crossings = 0;
            float previous = scene.heightAt(0, 10);
            check(previous < mla::world::waterLevel, "the profile starts on the lake bed");
            for (int step = 1; step <= 130; ++step) {
                const float height = scene.heightAt(0, static_cast<float>(10 + step));
                check(std::abs(height - previous) < .6f,
                      "the southern bank slopes into the water instead of stepping");
                crossings += (previous < mla::world::waterLevel) != (height < mla::world::waterLevel);
                previous = height;
            }
            check(crossings == 1, "the southern bank breaks the surface exactly once");
        }
    } catch (const std::exception& error) {
        std::cerr << "Unexpected failure: " << error.what() << '\n';
        return 1;
    }
    if (failures != 0) return 1;
    std::cout << "Heightmap interpolation, normals, boundaries and validation passed.\n";
    return 0;
}
