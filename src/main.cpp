#include "Camera.h"
#include "Scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace {
namespace fs = std::filesystem;

void printHelp() {
    std::cout << "\nMountain Lake & Aurora - dancing lights over the lake\n"
        << "WASD move | Space/Ctrl up/down | Shift faster | Mouse look | Wheel zoom\n"
        << "Tab capture/release mouse | [ / ] light direction | T pause animation\n"
        << "U aurora on/off | C aurora color: green / blue / orange\n"
        << "Up/Down aurora brightness | Left/Right aurora speed\n"
        << "M meteor shower on/off\n"
        << "L lighthouse beacon on/off | , / . sweep slower/faster\n"
        << "P wireframe | R scene / sky-only reflections | H console help | Esc quit\n"
        << "Current scene: terrain, starry sky, lake, aurora, meteors, pines, lighthouse and plane.\n"
        << "CLI: --smoke-test | --resources <folder> | --capture <frame.ppm>\n"
        << "     --aurora-color <green|blue|orange> | --help\n";
}

struct Options {
    bool smoke = false;
    fs::path resources;
    fs::path capture;
    mla::AuroraPalette auroraPalette = mla::AuroraPalette::Green;
};

Options parseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument(argv[i]);
        if (argument == "--smoke-test") options.smoke = true;
        else if (argument == "--aurora-color" && i + 1 < argc) {
            const std::string color(argv[++i]);
            if (color == "green") options.auroraPalette = mla::AuroraPalette::Green;
            else if (color == "blue") options.auroraPalette = mla::AuroraPalette::Blue;
            else if (color == "orange") options.auroraPalette = mla::AuroraPalette::Orange;
            else throw std::runtime_error("Aurora color must be green, blue or orange: " + color);
        }
        else if ((argument == "--resources" || argument == "--capture") && i + 1 < argc) {
            if (argument == "--resources") options.resources = fs::path(argv[++i]);
            else options.capture = fs::path(argv[++i]);
        } else throw std::runtime_error("Unknown or incomplete option: " + argument);
    }
    return options;
}

bool isResourceRoot(const fs::path& root) {
    return fs::is_regular_file(root / "shaders/terrain.vert") &&
           fs::is_regular_file(root / "assets/heightmaps/mountain_lake.pgm");
}

fs::path executablePath(const char* argument) {
#if defined(_WIN32)
    std::wstring buffer(32768, L'\0');
    const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && length < buffer.size()) return fs::path(buffer.substr(0, length));
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) == 0) return fs::weakly_canonical(buffer.data());
#else
    std::error_code error;
    auto path = fs::read_symlink("/proc/self/exe", error);
    if (!error) return path;
#endif
    return fs::absolute(argument);
}

fs::path resourceRoot(const Options& options, const char* executable) {
    if (!options.resources.empty()) {
        if (!isResourceRoot(options.resources))
            throw std::runtime_error("Invalid resource directory: " + options.resources.string());
        return fs::absolute(options.resources);
    }
    const std::vector<fs::path> candidates{executablePath(executable).parent_path(), fs::current_path()};
    for (const auto& candidate : candidates) if (isResourceRoot(candidate)) return candidate;
    throw std::runtime_error("Cannot find shaders/assets. Rebuild or use --resources <project folder>.");
}

// Declared before the scene so the context outlives every GPU resource.
class Window {
public:
    explicit Window(bool hidden) {
        glfwSetErrorCallback([](int code, const char* message) {
            std::cerr << "GLFW " << code << ": " << message << '\n';
        });
        if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);
        handle = glfwCreateWindow(1280, 720, "Mountain Lake & Aurora", nullptr, nullptr);
        if (!handle) {
            glfwTerminate();
            throw std::runtime_error("Cannot create an OpenGL 3.3 core window");
        }
        glfwMakeContextCurrent(handle);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            glfwDestroyWindow(handle);
            glfwTerminate();
            throw std::runtime_error("GLAD could not load OpenGL functions");
        }
        glfwSwapInterval(hidden ? 0 : 1);
    }
    ~Window() { glfwDestroyWindow(handle); glfwTerminate(); }
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    GLFWwindow* handle = nullptr;
};

struct Input {
    mla::Camera camera;
    mla::SceneSettings scene;
    bool captured = true;
    bool paused = false;
    bool firstMouse = true;
    double lastX = 0, lastY = 0;
};

Input& input(GLFWwindow* window) {
    return *static_cast<Input*>(glfwGetWindowUserPointer(window));
}

void attachInput(GLFWwindow* window, Input& state, bool smoke) {
    state.captured = !smoke;
    glfwSetWindowUserPointer(window, &state);
    glfwSetInputMode(window, GLFW_CURSOR, smoke ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
        auto& state = input(window);
        if (!state.captured) return;
        if (!state.firstMouse)
            state.camera.look(static_cast<float>(x - state.lastX), static_cast<float>(state.lastY - y));
        state.lastX = x; state.lastY = y; state.firstMouse = false;
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double, double y) {
        auto& state = input(window);
        if (state.captured) state.camera.zoom(static_cast<float>(y));
    });
    glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int) { input(window).firstMouse = true; });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int) {
        if (action != GLFW_PRESS) return;
        auto& state = input(window);
        switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_P: state.scene.wireframe = !state.scene.wireframe; break;
        case GLFW_KEY_R:
            state.scene.reflections = !state.scene.reflections;
            std::cout << "Reflections: " << (state.scene.reflections ? "full scene" : "sky only") << '\n';
            break;
        case GLFW_KEY_M:
            state.scene.meteors = !state.scene.meteors;
            std::cout << "Meteor shower: " << (state.scene.meteors ? "on" : "off") << '\n';
            break;
        case GLFW_KEY_U: state.scene.aurora = !state.scene.aurora; break;
        case GLFW_KEY_C:
            switch (state.scene.auroraPalette) {
            case mla::AuroraPalette::Green: state.scene.auroraPalette = mla::AuroraPalette::Blue; break;
            case mla::AuroraPalette::Blue: state.scene.auroraPalette = mla::AuroraPalette::Orange; break;
            case mla::AuroraPalette::Orange: state.scene.auroraPalette = mla::AuroraPalette::Green; break;
            }
            std::cout << "Aurora color: " << mla::auroraPaletteName(state.scene.auroraPalette) << '\n';
            break;
        case GLFW_KEY_T: state.paused = !state.paused; break;
        case GLFW_KEY_L: state.scene.beacon = !state.scene.beacon; break;
        case GLFW_KEY_H: printHelp(); break;
        case GLFW_KEY_TAB:
            state.captured = !state.captured; state.firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, state.captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            break;
        default: break;
        }
    });
}

void updateInput(GLFWwindow* window, Input& state, float dt) {
    if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)) return;
    auto down = [&](int key) { return glfwGetKey(window, key) == GLFW_PRESS ? 1.0f : 0.0f; };
    if (state.captured) {
        state.camera.move({down(GLFW_KEY_D) - down(GLFW_KEY_A),
                           down(GLFW_KEY_SPACE) - down(GLFW_KEY_LEFT_CONTROL),
                           down(GLFW_KEY_W) - down(GLFW_KEY_S)}, dt, down(GLFW_KEY_LEFT_SHIFT) > 0);
    }
    state.scene.lightAngle += dt * .5f * (down(GLFW_KEY_RIGHT_BRACKET) - down(GLFW_KEY_LEFT_BRACKET));
    state.scene.auroraIntensity = std::clamp(state.scene.auroraIntensity +
        dt * .4f * (down(GLFW_KEY_UP) - down(GLFW_KEY_DOWN)), 0.0f, 2.0f);
    state.scene.auroraSpeed = std::clamp(state.scene.auroraSpeed +
        dt * .3f * (down(GLFW_KEY_RIGHT) - down(GLFW_KEY_LEFT)), 0.1f, 2.5f);
    // Held, like the light direction. Zero is allowed: a stopped beam is the
    // easiest way to look at where the light lands.
    state.scene.beaconRpm = std::clamp(state.scene.beaconRpm +
        dt * 3.0f * (down(GLFW_KEY_PERIOD) - down(GLFW_KEY_COMMA)), 0.0f, 12.0f);
}

std::vector<unsigned char> readFrame(int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

void saveFrame(const fs::path& path, const std::vector<unsigned char>& pixels, int width, int height) {
    std::ofstream output(path, std::ios::binary);
    if (!output) throw std::runtime_error("Cannot write capture: " + path.string());
    output << "P6\n" << width << ' ' << height << "\n255\n";
    for (int row = height - 1; row >= 0; --row)
        output.write(reinterpret_cast<const char*>(pixels.data() + static_cast<std::size_t>(row) * width * 3), width * 3);
    if (!output) throw std::runtime_error("Failed writing capture: " + path.string());
}

void checkGraphicsErrors() {
    const auto error = glGetError();
    if (error != GL_NO_ERROR) throw std::runtime_error("OpenGL error: " + std::to_string(error));
}

int run(const Options& options, const char* executable) {
    const auto resources = resourceRoot(options, executable);
    Window window(options.smoke);
    std::cout << "OpenGL " << glGetString(GL_VERSION) << " | " << glGetString(GL_RENDERER)
              << "\nResources: " << resources.string() << '\n';
    Input state;
    state.scene.auroraPalette = options.auroraPalette;
    // Frame the tall aurora above the ridgeline while retaining the lake below.
    state.camera.look(0.0f, 50.0f);
    attachInput(window.handle, state, options.smoke);
    mla::Scene scene(resources);
    const auto lamp = scene.lighthouse().lampPosition();
    std::cout << "Lake surface: " << scene.lake().triangleCount() << " triangles\n"
              << "Lighthouse: " << scene.lighthouse().parts().size()
              << " sections, lamp at " << std::fixed << std::setprecision(1)
              << lamp.x << ", " << lamp.y << ", " << lamp.z << '\n'
              << std::defaultfloat;
    checkGraphicsErrors();
    printHelp();
    double previous = glfwGetTime(), titleTime = previous;
    unsigned frames = 0, titleFrames = 0;
    while (!glfwWindowShouldClose(window.handle)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = std::clamp(static_cast<float>(now - previous), 0.0f, 0.1f);
        previous = now;
        if (!options.smoke) updateInput(window.handle, state, dt);
        int width = 0, height = 0;
        glfwGetFramebufferSize(window.handle, &width, &height);
        if (width == 0 || height == 0) {
            if (options.smoke) throw std::runtime_error("Smoke test received a zero-size framebuffer");
            glfwWaitEventsTimeout(.05);
            continue;
        }
        // A fixed step keeps the smoke test's frames reproducible.
        const float animationStep = options.smoke ? 1.0f / 60.0f : (state.paused ? 0.0f : dt);
        state.scene.time += animationStep;
        state.scene.auroraTime += animationStep * state.scene.auroraSpeed;
        // The beam's phase is integrated rather than derived from the clock, so
        // changing the speed cannot jump it. Whole turns are dropped, which
        // keeps the number small however long the window stays open.
        state.scene.beaconTurns = std::fmod(state.scene.beaconTurns +
                                            animationStep * state.scene.beaconRpm / 60.0f, 1.0f);
        glViewport(0, 0, width, height);
        const auto background = mla::Scene::background();
        glClearColor(background.r, background.g, background.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.render(state.camera, static_cast<float>(width) / height, state.scene);
        ++frames; ++titleFrames;
        if (options.smoke || !options.capture.empty()) {
            const auto pixels = readFrame(width, height);
            std::size_t changedPixels = 0;
            for (std::size_t i = 0; i < pixels.size(); i += 3) {
                const int difference = std::abs(int(pixels[i]) - int(pixels[0])) +
                    std::abs(int(pixels[i + 1]) - int(pixels[1])) +
                    std::abs(int(pixels[i + 2]) - int(pixels[2]));
                if (difference > 20) ++changedPixels;
            }
            if (options.smoke && changedPixels < static_cast<std::size_t>(width) * height / 100)
                throw std::runtime_error("Smoke test rendered an unexpectedly uniform/dark frame");
            if (!options.capture.empty() && (!options.smoke || frames == 4))
                saveFrame(options.capture, pixels, width, height);
            checkGraphicsErrors();
        }
        glfwSwapBuffers(window.handle);
        if (options.smoke && frames == 4) {
            std::cout << "Graphics smoke test passed: 4 frames, shaders compiled, no OpenGL errors.\n";
            break;
        }
        if (!options.smoke && !options.capture.empty()) break;
        if (now - titleTime >= .5) {
            std::ostringstream title;
            title << "Mountain Lake & Aurora | " << std::fixed << std::setprecision(0)
                  << titleFrames / (now - titleTime) << " FPS | Reflections "
                  << (state.scene.reflections ? "scene" : "sky only")
                  << " | Meteors " << (state.scene.meteors ? "on" : "off") << " | Aurora "
                  << (state.scene.aurora ? "on" : "off") << " | "
                  << mla::auroraPaletteName(state.scene.auroraPalette) << " | Brightness "
                  << std::setprecision(0) << state.scene.auroraIntensity * 100.0f << "% | Speed "
                  << std::setprecision(1) << state.scene.auroraSpeed << "x"
                  << (state.paused ? " | Paused" : "");
            glfwSetWindowTitle(window.handle, title.str().c_str());
            titleTime = now; titleFrames = 0;
        }
    }
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    try {
        for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--help") { printHelp(); return 0; }
        return run(parseOptions(argc, argv), argv[0]);
    } catch (const std::exception& error) {
        std::cerr << "Mountain Lake & Aurora: " << error.what() << '\n';
        return 1;
    }
}
