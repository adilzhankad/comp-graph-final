# Bundled source dependencies

All third-party inputs for the normal build are kept in this directory. CMake compiles GLFW and the generated GLAD C source with the same compiler/configuration as the application. GLM is header-only; stb_image's implementation is compiled once in `src/ImageLoader.cpp`.

Do not copy machine-specific `.lib`, `.dll`, `.a`, `.obj` or `.o` files into the repository. Generated build outputs belong under the ignored build directory.

| Directory | Upstream version | Use |
|---|---|---|
| `glfw/` | [GLFW 3.4](https://github.com/glfw/glfw/releases/tag/3.4) | Window creation, OpenGL context and input |
| `glad/` | [GLAD 0.1.36](https://github.com/Dav1dde/glad/tree/v0.1.36), generated for OpenGL 3.3 core | OpenGL function loader; `glad/glad.h` and `gladLoadGLLoader` API |
| `glm/` | [GLM 1.0.1](https://github.com/g-truc/glm/releases/tag/1.0.1) | Header-only vector/matrix mathematics |
| `stb/` | Commit [`003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5`](https://github.com/nothings/stb/tree/003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5) | `stb_image.h` for image/heightmap decoding |

[`versions.json`](versions.json) records exact download URLs, versions/revisions and SHA-256 hashes. GLFW, GLM and GLAD hashes identify their downloaded source archives; the stb hash identifies `stb_image.h`. Keep source and headers from the same version together. GLAD 2 uses a different API and requires corresponding application changes.

## GLAD generation

The checked-in loader was generated from the GLAD `v0.1.36` source archive, using its packaged Khronos specification. From that generator's source directory, run:

```text
python -S -X utf8 -m glad --profile core --api gl=3.3 --generator c --spec gl --extensions= --reproducible --out-path <project>/external/glad
```

Replace `<project>` with the destination project path. `-S` uses Python's standard-library XML parser rather than an installed third-party parser; `-X utf8` avoids Windows locale-dependent decoding. `--reproducible` uses the bundled registry rather than fetching a newer specification. Preserve the matching `glad.c`, `glad/glad.h`, `KHR/khrplatform.h` and upstream `LICENSE` together. The generated header records the API/profile configuration.

Normal users need a C++ toolchain and CMake, plus platform development packages on Linux. They do not need Python, Git submodule initialization, a package manager or network access after cloning the full repository. Python is only needed to regenerate the project's heightmap, or to regenerate GLAD during a deliberate dependency update.

Preserve license files and update [CREDITS.md](../CREDITS.md) when changing a dependency. Dear ImGui is not bundled at this stage.
