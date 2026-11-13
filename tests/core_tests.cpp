#include "Beacon.h"
#include "Camera.h"
#include "SceneNode.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

bool near(glm::vec3 a, glm::vec3 b, float epsilon = 0.0001f) {
    return glm::length(a - b) < epsilon;
}

glm::vec3 facing(const mla::Camera& camera) {
    return -glm::vec3(glm::inverse(camera.view())[2]);
}

template <typename Function>
void requireInvalid(Function function, const std::string& message) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error(message);
}

void cameraMovement() {
    mla::Camera camera;
    const auto origin = camera.position;
    require(near(origin, glm::vec3(0.0f, 9.0f, 36.0f)), "Unexpected camera start position");
    require(near(glm::vec3(camera.view() * glm::vec4(origin, 1.0f)), glm::vec3(0.0f)),
            "View matrix does not map camera position to origin");
    const glm::vec3 front = facing(camera);
    require(front.z < -0.98f && front.y < 0.0f, "Camera must initially face forward and slightly down");
    camera.move({0.0f, 0.0f, 1.0f}, 0.5f);
    require(near(camera.position - origin, front * 4.0f), "Forward movement does not follow viewing direction");

    mla::Camera right;
    right.move({1.0f, 0.0f, 0.0f}, 0.5f);
    require(near(right.position - origin, glm::vec3(4.0f, 0.0f, 0.0f)), "Right movement has incorrect axis");
    mla::Camera up;
    up.move({0.0f, 1.0f, 0.0f}, 0.5f);
    require(near(up.position - origin, glm::vec3(0.0f, 4.0f, 0.0f)), "Vertical movement must follow world up");
    mla::Camera diagonal;
    diagonal.move({1.0f, 0.0f, 1.0f}, 0.5f);
    require(near(glm::length(diagonal.position - origin), 4.0f), "Diagonal movement increases speed");
    mla::Camera fast;
    fast.move({0.0f, 0.0f, 1.0f}, 0.5f, true);
    require(near(glm::length(fast.position - origin), 12.0f), "Fast camera movement is incorrect");
    mla::Camera split;
    split.move({0.0f, 0.0f, 1.0f}, 0.25f);
    split.move({0.0f, 0.0f, 1.0f}, 0.25f);
    require(near(split.position, camera.position), "Movement depends on frame count");
    const auto before = camera.position;
    camera.move({0.0f, 0.0f, 0.0f}, 0.5f);
    camera.move({1.0f, 1.0f, 1.0f}, 0.0f);
    require(near(camera.position, before), "Zero input or zero elapsed time moves the camera");
    requireInvalid([&] { camera.move({0.0f, 0.0f, 1.0f}, -1.0f); }, "Negative dt was accepted");
}

void cameraLookAndProjection() {
    mla::Camera camera;
    camera.look(900.0f, 20.0f);
    require(near(facing(camera), glm::vec3(1.0f, 0.0f, 0.0f)), "Mouse look has incorrect yaw or pitch sign");
    camera.look(0.0f, 100000.0f);
    const auto up = facing(camera);
    require(up.y > 0.99f && up.x > 0.0f, "Upper pitch clamp fails or flips the camera");
    camera.look(0.0f, -200000.0f);
    require(facing(camera).y < -0.99f, "Lower pitch clamp fails");

    mla::Camera projectionCamera;
    const auto square = projectionCamera.projection(1.0f);
    const auto wide = projectionCamera.projection(2.0f);
    require(near(wide[0][0], square[0][0] * 0.5f) && near(wide[1][1], square[1][1]),
            "Projection does not preserve vertical field of view when aspect changes");
    projectionCamera.zoom(5.0f);
    require(projectionCamera.projection(1.0f)[1][1] > square[1][1], "Positive scroll should zoom in");
    projectionCamera.zoom(10000.0f);
    const auto narrow = projectionCamera.projection(1.0f);
    projectionCamera.zoom(10000.0f);
    require(near(narrow[1][1], projectionCamera.projection(1.0f)[1][1]), "Minimum field of view is not clamped");
    projectionCamera.zoom(-10000.0f);
    const auto broad = projectionCamera.projection(1.0f);
    projectionCamera.zoom(-10000.0f);
    require(near(broad[1][1], projectionCamera.projection(1.0f)[1][1]), "Maximum field of view is not clamped");
    require(broad[1][1] < square[1][1], "Zoom out should widen field of view");
    requireInvalid([&] { projectionCamera.projection(0.0f); }, "Zero aspect ratio was accepted");
    requireInvalid([&] { projectionCamera.projection(std::numeric_limits<float>::infinity()); },
                   "Non-finite aspect ratio was accepted");
}

void sceneHierarchy() {
    mla::SceneNode root;
    root.local = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)) *
                 glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto child = std::make_unique<mla::SceneNode>();
    child->local = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    auto& childRef = root.addChild(std::move(child));
    auto grandchild = std::make_unique<mla::SceneNode>();
    grandchild->local = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    auto& grandchildRef = childRef.addChild(std::move(grandchild));
    root.updateWorld();
    require(near(glm::vec3(childRef.world[3]), glm::vec3(3.0f, 2.0f, 0.0f)),
            "Child translation does not inherit parent rotation");
    require(near(glm::vec3(grandchildRef.world[3]), glm::vec3(2.0f, 2.0f, 0.0f)),
            "Grandchild world transform does not inherit both ancestors");
    root.local = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    root.updateWorld(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f)));
    require(near(glm::vec3(grandchildRef.world[3]), glm::vec3(1.0f, 1.0f, 5.0f)),
            "Changed transforms or external parent matrix are not propagated");
    requireInvalid([&] { root.addChild(nullptr); }, "Null child was accepted");
    root.children.push_back(nullptr);
    root.updateWorld();
    require(near(glm::vec3(childRef.world[3]), glm::vec3(1.0f, 0.0f, 0.0f)),
            "Empty child slots interfere with hierarchy updates");
}

void beaconSweep() {
    // The same hierarchy the lighthouse builds: the lamp hangs under the tower,
    // raised along its axis and turned about it. Nothing else moves the beam.
    constexpr float tau = 6.28318530718f;
    const glm::vec3 cone(0.9917f, 0.9994f, 170.0f);
    mla::SceneNode tower;
    tower.local = glm::translate(glm::mat4(1.0f), glm::vec3(29.0f, 1.0f, -5.0f));
    auto& lamp = tower.addChild(std::make_unique<mla::SceneNode>());
    auto sweep = [&](float turns) {
        lamp.local = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 17.0f, 0.0f)),
                                 turns * tau, glm::vec3(0.0f, 1.0f, 0.0f));
        tower.updateWorld();
        return mla::beaconFromLamp(lamp.world, glm::vec3(2.1f, 1.7f, 1.1f), cone);
    };

    const auto start = sweep(0.0f);
    require(near(start.position, glm::vec3(29.0f, 18.0f, -5.0f)), "Beacon is not at the lamp");
    require(near(start.direction, glm::vec3(0.0f, 0.0f, 1.0f)),
            "The beam must leave the lamp along its local +z");
    const auto quarter = sweep(0.25f);
    require(near(quarter.position, start.position), "Turning the lamp must not move it");
    require(near(quarter.direction, glm::vec3(1.0f, 0.0f, 0.0f)),
            "A quarter turn must sweep the beam by 90 degrees");
    require(near(sweep(1.0f).direction, start.direction),
            "A whole turn must bring the beam back to its start");
    const auto partial = sweep(0.37f);
    require(near(glm::length(partial.direction), 1.0f), "Beam direction must stay normalized");
    require(near(partial.direction.y, 0.0f), "A beam swept about the tower axis must stay level");
    require(near(start.color, glm::vec3(2.1f, 1.7f, 1.1f)) && near(start.cone, cone),
            "Beacon colour and cone are not passed through");
    requireInvalid([&] { mla::beaconFromLamp(glm::mat4(0.0f), glm::vec3(1.0f), cone); },
                   "A lamp transform with no orientation was accepted");
    requireInvalid([&] { mla::beaconFromLamp(glm::mat4(1.0f), glm::vec3(1.0f),
                                             glm::vec3(0.999f, 0.99f, 170.0f)); },
                   "A cone whose core is wider than its edge was accepted");
    requireInvalid([&] { mla::beaconFromLamp(glm::mat4(1.0f), glm::vec3(1.0f),
                                             glm::vec3(0.99f, 0.999f, 0.0f)); },
                   "A beacon with no range was accepted");
}

} // namespace

int main() {
    try {
        cameraMovement();
        cameraLookAndProjection();
        sceneHierarchy();
        beaconSweep();
        std::cout << "Camera, scene hierarchy and beacon tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Core test failure: " << error.what() << '\n';
        return 1;
    }
}
