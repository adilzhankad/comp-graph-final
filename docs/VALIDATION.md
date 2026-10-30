# Terrain and fog validation

Updated 2026-10-31. The current scene renders only the low-poly terrain. Earlier lake, aurora and prop modules are retained as future prototypes.

Latest terrain revision: seven connected ridge chains and twelve descending spurs replace the isolated peak envelopes. The visible starting skyline now has elevated saddles and shared flanks. The regenerated heightmap passed the existing heightmap test executable, and a visible-window OpenGL capture was checked visually and passed the application's error checks. `preview.png` records this revision. C++ and shader source did not change in this pass; the build results below remain the preceding code baseline.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.44 |
| CPU tests | Camera/hierarchy and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Visible-window capture completed; terrain shaders compiled/linked and the OpenGL error check passed |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3.0 / driver32.0.101.6913 |
| World | 560x560 units; 513x513 image samples; 128x128 cells / 32,768 triangles |
| Low-poly geometry | Split triangle vertices with flat geometric normals; jittered shared positions and mixed diagonals |
| Fog | Smooth distance fade from95 to230 units; boundary fade from205 to260 on each axis before the mesh ends at280 |

`preview.png` is the current application capture. Fog is fully opaque and exactly matches the background before reaching the outer boundary, including when approaching it. Face lighting remains flat while fog interpolates smoothly.

No performance benchmark or cross-platform runtime result is claimed. Interactive exploration and GitHub CI remain team review tasks. Python was used only to regenerate the heightmap and convert the preview; ordinary builds still need no Python or downloads.
