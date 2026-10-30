cmake_minimum_required(VERSION 3.16)
project(MountainLakeAurora VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Every dependency is source code checked into external/. No downloads at configure time.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(external/glfw EXCLUDE_FROM_ALL)

add_library(mla_glad STATIC external/glad/src/glad.c)
target_include_directories(mla_glad SYSTEM PUBLIC external/glad/include)
target_link_libraries(mla_glad PUBLIC ${CMAKE_DL_LIBS})
add_library(mla_glm INTERFACE)
target_include_directories(mla_glm SYSTEM INTERFACE external/glm)
add_library(mla_image STATIC src/ImageLoader.cpp)
target_include_directories(mla_image SYSTEM PUBLIC external/stb)

add_library(mla_core STATIC src/Camera.cpp src/SceneNode.cpp src/Heightmap.cpp)
target_include_directories(mla_core PUBLIC src)
target_link_libraries(mla_core PUBLIC mla_glm mla_image)

add_executable(MountainLakeAurora
    src/main.cpp src/Shader.cpp src/Mesh.cpp src/Terrain.cpp
    src/Scene.cpp)
target_link_libraries(MountainLakeAurora PRIVATE mla_core mla_glad glfw)
target_compile_definitions(MountainLakeAurora PRIVATE GLFW_INCLUDE_NONE NOMINMAX)
set_property(TARGET MountainLakeAurora PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MountainLakeAurora>")
if(CMAKE_CONFIGURATION_TYPES)
    set(MLA_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
else()
    set(MLA_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()
set_property(TARGET MountainLakeAurora PROPERTY RUNTIME_OUTPUT_DIRECTORY "${MLA_RUNTIME_DIR}")
foreach(target mla_core MountainLakeAurora)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

# An always-run copy target also picks up shader-only changes without a relink.
add_custom_target(mla_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${MLA_RUNTIME_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${MLA_RUNTIME_DIR}/assets"
    COMMENT "Copying shaders and assets beside the executable")
add_dependencies(MountainLakeAurora mla_resources)

include(CTest)
if(BUILD_TESTING)
    add_executable(core_tests tests/core_tests.cpp)
    target_link_libraries(core_tests PRIVATE mla_core)
    add_test(NAME camera_and_hierarchy COMMAND core_tests)
    add_executable(heightmap_tests tests/heightmap_tests.cpp)
    target_link_libraries(heightmap_tests PRIVATE mla_core)
    add_test(NAME heightmap COMMAND heightmap_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/heightmaps/mountain_lake.pgm")
endif()
